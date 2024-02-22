/******************************************************************************
Title:    encoder -> PWM
Author:   rue_mohr
Date:     Aug 19 2005
Software: AVR-GCC 3.3 
Hardware: atmega32
  

                 
+-------------------------------------+
 |                          Sig Vd Gnd |
 |  +---------+   5V O     PB0 [o o o] | 
 |  | 7805  O |   Vd O     PB1 [o o o] | 
 |  +---------+   V+ .     PB2 [o o o] 10 | -> dir
 |                         PB3 [o o o] 12 | -> pwm
 |                         PB4 [o o o] 11 | 
 |                         PB5 [o o o] | 
 |                         PB6 [o o o] | 
 |                         PB7 [o o o] | 
 |                         PA0 [o o o] | 
 |                         PA1 [o o o] | 
 |        +----------+     PA2 [o o o] | 
 |        |O         |     PA3 [o o o] | 
 |        |          |     PA4 [o o o] | 
 |        |          |     PA5 [o o o] | 
 |        |          |     PA6 [o o o] | 
 |        |          |     PA7 [o o o] | 
 |        |          |     PC7 [o o o] |
 |        |          |     PC6 [o o o] |
 |        |          |     PC5 [o o o] |
 |        | ATMEGA32 |     PC4 [o o o] |
 |        |          |     PC3 [o o o] |
 |        |          |     PC2 [o o o] |
 |        |          |     PC1 [o o o] A1 | <- down button (dir)
 |        |          |     PC0 [o o o] A0 | <- up button (step)
 |        |          |     PD7 [o o o] 7  | -> loop heartbeat
 |        |          |     PD2 [o o o] 2  | <- channel A
 |        |          |     PD3 [o o o] 3  | <- channel B 
 |        |          |     PD4 [o o o] 4  |
 |        |          |     PD5 [o o o] 5  |
 |        +----------+     PD6 [o o o] 6  |
 |      E.D.S BABYBOARD III               |
 +-------------------------------------+


what this should do:
3000 line encoder, this shouls measeure the speed, in degrees/second
    
*******************************************************************************/
 
/****************************| INCLUDGEABLES |********************************/
 
#include<avr/io.h>
#include <avr/interrupt.h>
//#include <avr/signal.h>
#include "usart.h"
#include "avrcommon.h"
#include "nopDelay.h"
#include "hiComms2.h"
#include "localsys.h"
 
/*****************************| DEFINIATIONS |********************************/
 


 
 
/*****************************| VARIABLES |********************************/

 
 
volatile unsigned int  position;           // note that this is signed              
volatile unsigned int  direction;  
volatile unsigned int  moveTicks;                                                 
volatile char          flag;

 
#define ERROR 0
signed char offsets[] = {
        /* 0000 */   0,
        /* 0001 */  +1,
        /* 0010 */  -1,
        /* 0011 */   ERROR,
        /* 0100 */  -1,
        /* 0101 */   0,
        /* 0110 */   ERROR,
        /* 0111 */  +1,
        /* 1000 */  +1,
        /* 1001 */   ERROR,
        /* 1010 */   0,
        /* 1011 */  -1,
        /* 1100 */   ERROR,
        /* 1101 */  -1,
        /* 1110 */  +1,
        /* 1111 */   0
};
 
/************************| FUNCTION DECLARATIONS |*************************/
 
void         setupEncoder ( void );
void         updatePos    ( void );
void         speed_init   ( void );
  
/****************************| CODE... |***********************************/
 

int main (void)  {  
  
  unsigned int targetHit, targetSpeed;
  unsigned int stopPos;
  unsigned int target;
  unsigned int x, v;
  
  
  DDRB = (INPUT << PB0 | INPUT << PB1 |OUTPUT << PB2 |OUTPUT << PB3 |INPUT << PB4 |INPUT << PB5 |INPUT << PB6 |INPUT << PB7);
  DDRC = (INPUT << PC0 | INPUT << PC1 |INPUT << PC2 |INPUT << PC3 |INPUT << PC4 |INPUT << PC5 |INPUT << PC6 );
  DDRD = (INPUT << PD0 | INPUT << PD1 |INPUT << PD2 |INPUT << PD3 |INPUT << PD4 |INPUT << PD5 |INPUT << PD6 |OUTPUT << PD7);        
  
  PORTC = 0xFF ; // turn on pullups.      
  
  USART_Init( 16 ); // 57600
  
  speed_init() ;
  setupEncoder(); 
  MotorOff();
  Delay(724637); // 2 second delay
  Delay(724637);
  sei();
  

  position = 0;
  target   = 7; //about 0.9 degrees
  
  USART_printstring( "Run, Hit, HitSpeed, StopPos \n");
  
  // sweep run position from 18 lines to 7200 lines.
  
   while(target < 2881) {
   
     position = 0;

     // turn on forward drive
     MotorForward();

     // at target line count switch to reverse direction
     x = 0;
     v = 255;
     while( x < target) { 
       for(flag = 0; !flag; );
       cli(); 
         x = position; 
         v = moveTicks; 
       sei();
       v >>= VELOCITYSCALE;
       x += VLUT[v] ;              
     }
     targetHit   = position;   
     targetSpeed = moveTicks;
     MotorReverse();
       

     // monitor direction, as soon as it reverses, cut power
     while(direction != -1);
     MotorOff();
     stopPos = position;

     // sleep 0.5 seconds
     printNumDec16( target );
     USART_printstring( ", ");
     printNumDec16( targetHit );
     USART_printstring( ", ");
     printNumDec16( targetSpeed );     
     USART_printstring( ", ");
     printNumDec16( stopPos );
     USART_printstring( "\n");   

     Delay(724637); // allow a new ns to send the serial :]
     target++;
      
   }
   
   while(1); 
   
}
    
//------------------------| FUNCTIONS |------------------------
 

 
//-------------------------- QUADRATURE ENCODER --------------------------





ISR(TIMER1_OVF_vect) {
  moveTicks = 60000;
  direction = 0;
}



void speed_init() {
   TCNT1  = 0;
   TCCR1B =   0 << CS12  | /* prescale by 1*/
              0 << CS11  |
              1 << CS10  ;
 }





// ------------ position  STUFF ----------------------------

void setupEncoder() {        
  position = 0;   // clear position 
  EICRA = (1<<ISC10) | (1<<ISC00);  // we need to set up int0 and int1 to trigger interrupts on both edges         
  EIMSK  = (1<<INT0) | (1<<INT1); // then enable them.
}
 
// SIGNAL (SIG_INTERRUPT0) {  // fix this signal name!!! INT0
ISR(INT0_vect){
   updatePos();
}

// SIGNAL (SIG_INTERRUPT1) { // fix this signal name!!! INT1
ISR(INT1_vect){ 
   updatePos();
}
 
void updatePos() {
  static unsigned char oldstate;
  signed char          t;

  
/*      get the bits        PIND     = XXXXiiXX
        clear space       & 0x0C     = 0000ii00
        or in old status  | oldstate = XX00iijj
        clean up          & 0x0F     = 0000iijj  */  
     	   
       
  oldstate = oldstate | (PIND & 0x0C); // Update Oldstate   
  t = offsets[oldstate];
  oldstate = oldstate >> 2; 
  
  position += t;       // Update Position

  if ((direction == t) || (direction == 0)) { // if the direction is the same as last time *** important correction here ****
    moveTicks = TCNT1;
  } else  {
    moveTicks = 60000; // "0" if we changed direction and dont have a new reading yet.
    t = 0;             // set direction to zero. (average, we just turned around)
  }

  TCNT1     = 0;
  direction = t;      	      
  flag++;	
}



































