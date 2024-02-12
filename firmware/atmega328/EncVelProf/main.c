/******************************************************************************
Title:    velocity profiler for EGMK
Author:   rue_mohr
Date:     Aug 19 2005
Software: AVR-GCC 3.3 
Hardware: atmega328
  


what this should do:

This performs position moves from 2 to 4000 lines and records the system performance.
The goal is NOT to hit the target, its to find out what the stopping distances are,
we dont really care what the origional move was, we care about the velocity, 
The change in target is used to try to sample various system speeds.

    
*******************************************************************************/
 
/****************************| INCLUDGEABLES |********************************/
 
#include<avr/io.h>
#include <avr/interrupt.h>
//#include <avr/signal.h>
#include "usart.h"
#include "avrcommon.h"
#include "nopDelay.h"
#include "hiComms2.h"
 
/*****************************| DEFINIATIONS |********************************/
 

 
#define OUTPUT             1
#define INPUT              0

 


// power     is PB3
// direction is PB2

 #define  MotorForward()  SetBit(2, PORTB) ;    SetBit(3, PORTB)    
 #define  MotorReverse()  ClearBit(2, PORTB);   SetBit(3, PORTB)  
 #define  MotorOff()      ClearBit(2, PORTB) ;  ClearBit(3, PORTB)  
 
 
 
/*****************************| VARIABLES |********************************/

 
 
volatile unsigned int  position;           // note that this is signed              
volatile char  direction;  
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
  
  unsigned int targetHit;
  unsigned int targetSpeed;
  unsigned int stopPos;
  unsigned int coastPos;
  unsigned int target;
  unsigned int t;
  
  
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
  target   = 2;
  
  USART_printstring( "Target, Hit, HitSpeed, StopPos, coastPos \n");
  
  // sweep run position from 2 lines to 4000 lines.
  
   while(target < 4000) {
   
     position = 0;

     // turn on forward drive
     MotorForward();

     // at target line count switch to reverse direction
     t = position;
     while( t < target) { flag = 0; while(!flag); cli(); t = position; sei(); }
     targetHit   = t;   
     targetSpeed = moveTicks;
     MotorReverse();
       
       

     // monitor direction, as soon as it reverses, cut power
     while(direction != -1);
     MotorOff();
     
     stopPos = position;
     
     Delay(724637); // settle.2 seconds.
     
     coastPos = position;

     
     printNumDec16( target );
     USART_printstring( ", ");
     printNumDec16( targetHit );
     USART_printstring( ", ");
     printNumDec16( targetSpeed );     
     USART_printstring( ", ");
     printNumDec16( stopPos );
     USART_printstring( ", ");
     printNumDec16( coastPos );     
     USART_printstring( "\n");   
     
     target++;
      
   }
   
   while(1); 
   
}
    
//------------------------| FUNCTIONS |------------------------
 

 
//-------------------------- QUADRATURE ENCODER --------------------------





ISR(TIMER1_OVF_vect) {
  moveTicks = 60000;
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
  
  if (t == 0) return; // if there was a glitch igmore.
  
  moveTicks = TCNT1;   TCNT1 = 0;
  
  position += t;       // Update Position

  if (direction != t) { // if the direction is the same
    moveTicks = 60000; // "0" if we changed direction and dont have a new reading yet.
  }  

  direction = t;      	      
  flag++;	
}



































