/******************************************************************************
Title:    encoder -> serial
Author:   rue_mohr
Date:     Aug 19 2005
Software: AVR-GCC 3.3 
Hardware: atmega32
  


what this should do:
  show encoder position to serial port
    
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

 
// encoder on PD2, PD3

 #define  MotorForward()  SetBit(2, PORTB) ;    SetBit(3, PORTB)    
 #define  MotorReverse()  ClearBit(2, PORTB);   SetBit(3, PORTB)  
 #define  MotorOff()      ClearBit(2, PORTB) ;  ClearBit(3, PORTB)  
 
 
 
/*****************************| VARIABLES |********************************/

 
 
volatile unsigned int  position;           // note that this is signed              
volatile unsigned int  direction;                                                   
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

  
/****************************| CODE... |***********************************/
 

int main (void)  {  
  
  unsigned int targetHit;
  unsigned int stopPos;
  unsigned int target;
  unsigned int t;
  
  
  DDRB = (INPUT << PB0 | INPUT << PB1 |OUTPUT << PB2 |OUTPUT << PB3 |INPUT << PB4 |INPUT << PB5 |INPUT << PB6 |INPUT << PB7);
  DDRC = (INPUT << PC0 | INPUT << PC1 |INPUT << PC2 |INPUT << PC3 |INPUT << PC4 |INPUT << PC5 |INPUT << PC6 );
  DDRD = (INPUT << PD0 | INPUT << PD1 |INPUT << PD2 |INPUT << PD3 |INPUT << PD4 |INPUT << PD5 |INPUT << PD6 |OUTPUT << PD7);        
  
  PORTC = 0xFF ; // turn on pullups.      
  
  USART_Init( 16 ); // 57600
  
  setupEncoder(); 
  MotorOff();

  sei();

  position = 0;
  target   = 7; //about 0.9 degrees
  
  USART_printstring( "Target, Hit, StopPos \n");
  
  // sweep run position from 18 lines to 7200 lines.
  
   position = 0;
  
   while(1) {
       
     while(!flag); flag = 0;
     
     target = position;     
     
     printNumDec16( target );
      
     USART_printstring( "\n");   
      
   }
   
   while(1); 
   
}
    
//------------------------| FUNCTIONS |------------------------
 

 
//-------------------------- QUADRATURE ENCODER --------------------------

// SIGNAL (SIG_INTERRUPT0) {  // fix this signal name!!! INT0
ISR(INT0_vect){
   updatePos();
}

// SIGNAL (SIG_INTERRUPT1) { // fix this signal name!!! INT1
ISR(INT1_vect){ 
   updatePos();
}
 
 
 
 
 
void setupEncoder() {
        // clear position
        position = 0; 
 
        // we need to set up int0 and int1 to 
        // trigger interrupts on both edges
        EICRA = (1<<ISC10) | (1<<ISC00);
 
        // then enable them.
        EIMSK  = (1<<INT0) | (1<<INT1);
}
 
 
 

 
void updatePos() {
  static unsigned char oldstate;

  
/*      get the bits        PIND     = XXXXiiXX
        clear space       & 0x0C     = 0000ii00
        or in old status  | oldstate = XX00iijj
        clean up          & 0x0F     = 0000iijj  */  
     	   
       
  oldstate = oldstate | (PIND & 0x0C); // Update Oldstate 
  position += offsets[oldstate];       // Update Position

  direction = offsets[oldstate];

  oldstate = oldstate >> 2; 
    	      
  flag++;	
}



































