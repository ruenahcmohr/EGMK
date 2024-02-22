
/******************************************************************************
Title:    EGMK DC stepped servo
Author:   rue_mohr
Date:     Aug 19 2005
Software: AVR-GCC 3.3 
Hardware: atmega32
  

                 
		                                                          
pins:
  
  on pro mini:

0   rxd    rxd
1   txd    txd

2   pd2   encoder A 
3   pd3   encoder B
4   pd4    
5   pd5    
6   pd6  
7   pd7   

8   pb0    
9   pb1  step 
10  pb2  motor driver  
11  pb4  motor driver  
12  pb3    
13  pb5   


A0  pc0   
A1  pc1    
A2  pc2    
A3  pc3    
A4  pc4    
A5  pc5    
A6  adc6
A7  adc7
                  
		 
		 
		 

what this should do:

Should step position on each signal to PB1 trigger

    
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

 
 
volatile int  position;           // note that this is signed              
volatile char direction;  
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
  
  int targetHit, targetSpeed, stopPos;
  int loop;
  
 // int step;
  unsigned int v;
  int x;
  unsigned char dir;
  unsigned char t, ot;
  
  
  DDRB = (INPUT << PB0 | INPUT << PB1 |OUTPUT << PB2 |OUTPUT << PB3 |INPUT << PB4 |INPUT << PB5 |INPUT << PB6 |INPUT << PB7);
  DDRC = (INPUT << PC0 | INPUT << PC1 |INPUT << PC2 |INPUT << PC3 |INPUT << PC4 |INPUT << PC5 |INPUT << PC6 );
  DDRD = (INPUT << PD0 | INPUT << PD1 |INPUT << PD2 |INPUT << PD3 |INPUT << PD4 |INPUT << PD5 |INPUT << PD6 |INPUT << PD7);        
  
  PORTC = 0xFF ; // turn on pullups.      
  
  USART_Init( 16 ); // 57600
  
  speed_init() ;
  setupEncoder(); 
  MotorOff();
  Delay(724637); // 2 second startup delay
  Delay(724637);
  position = 0;
  
  sei();
  
 // step     =   1260; // 144 72 36
  
  USART_printstring( "Run, Hit, HitSpeed, StopPos \n"); 
     
 // while(1) {
   for (loop = 0; loop < 6000;) {
   
     ot = t; t = IsHigh(7, PIND);
     
     if ((ot == 0) && (t == 1))  {
       cli();   position += step; sei();
       loop++;
     }
          
     if ((position < 20) && (position > -20)) continue;  // wait untill we have enough error to deal with by velocity
	  
     dir = (position<0)?1:0;
     
     if (dir == 1) {
       MotorForward(); // turn on forward drive

       do {
	 for(flag = 0; !flag; );
	 cli(); 
           x = position; 
           v = moveTicks; 
	 sei();

         targetHit   = x;

	 v >>= VELOCITYSCALE;       // velocity scaled down to fit 8 bit table.
	 if (v > 255) v = 255;
         targetSpeed = v;       

	 x += FVLUT[v] ; // anticipate stop position              

       } while( x < 0);

       MotorReverse();

      // while(moveTicks < 3000);   // wait for it to basically come to a stop, as best we can measure.
      while(direction != -1);
       
     } else {

       MotorReverse(); // turn on forward drive

       do {
	 for(flag = 0; !flag; );
	 cli(); 
           x = position; 
           v = moveTicks; 
	 sei();

         targetHit   = x;

	 v >>= VELOCITYSCALE;       // velocity scaled down to fit 8 bit table.
	 if (v > 255) v = 255;
         targetSpeed = v;     
	 
	 x -= RVLUT[v] ; // anticipate stop position              

       } while( x > 0);

       MotorForward();

       //while(moveTicks < 3000);   // wait for it to basically come to a stop, as best we can measure.
       while(direction != 1);
     
     }
     
     MotorOff();
     
     stopPos = position;
     
     Delay(2000);     
     
     if (dir) {
       FVLUT[targetSpeed] = (FVLUT[targetSpeed]+stopPos)/2;          
     } else {
       RVLUT[targetSpeed] = (FVLUT[targetSpeed]+stopPos)/2; 
     }      
   
  }
  
    USART_printstring( "index , fval, rval \n");

    for (loop = 0; loop < 256; loop++) {
      printSignNumDec16( loop );
      USART_printstring( ",");
      printNumDec16( FVLUT[loop] );  
      USART_printstring( ",");
      printNumDec16( RVLUT[loop] );       
      USART_printstring( "\n");   
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
  
  if (t == 0) return; // ignore glitch pulses
  
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



































