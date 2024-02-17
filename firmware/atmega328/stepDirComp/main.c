/******************************************************************************
Title:    step/dir -> serial count
Author:   rue_mohr
Date:     Aug 19 2005
Software: AVR-GCC 3.3 
Hardware: atmega328
  

+ side of comparitor set to bandgap
Step input on: AIN1 (PD7)
Dir input on:  AIN0 (PD6)


    pins:
  
  on pro mini:

0   rxd    rxd
1   txd    txd

2   pd2    
3   pd3    
4   pd4    
5   pd5    
6   pd6  DIR
7   pd7  STEP  

8   pb0    
9   pb1   
10  pb2    
11  pb4    
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
 take the step dir and output a count on PD7
    
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

 
 
volatile int  position;           // note that this is signed              

 
/************************| FUNCTION DECLARATIONS |*************************/
 
void         setupEncoder ( void );
void         updatePos    ( void );
void         speed_init   ( void );
void         setupStepDir ( void );
  
  
/****************************| CODE... |***********************************/
 
 

int main (void)  {  
  
  
  DDRB = (INPUT << PB0 | INPUT << PB1 |OUTPUT << PB2 |OUTPUT << PB3 |INPUT << PB4 |INPUT << PB5 |INPUT << PB6 |INPUT << PB7);
  DDRC = (INPUT << PC0 | INPUT << PC1 |INPUT << PC2 |INPUT << PC3 |INPUT << PC4 |INPUT << PC5 |INPUT << PC6 );
  DDRD = (INPUT << PD0 | INPUT << PD1 |INPUT << PD2 |INPUT << PD3 |INPUT << PD4 |INPUT << PD5 |INPUT << PD6 |INPUT << PD7);        
  
  PORTC = 0xFF ; // turn on pullups.      
  
  USART_Init( 16 ); // 57600
  
  setupStepDir(); 
 
  
  sei();
  
  position = 0;

  
  USART_printstring( "Ready! \n");
  
   while(1) {

     printSignNumDec16( position);
     USART_printstring( "\n");   
      
   }
   
}
    
//------------------------| FUNCTIONS |------------------------
 
void setupStepDir() {

/*
  ACME   of ADCSRB = 0; // negative input is AIN1
  SCIS0  of ACSR   = 1; // interrupt on comparator rising edge 
  SCIS1  of ACSR   = 1; // "
  ACBG   of ACSR   = 1; // use the badgap for + input
  ACIE   of ACSR   = 1; // enable comparator interrupt

  
*/

  ACSR   = ( 1 << ACIS1 ) | ( 1 << ACBG ) | ( 1 << ACIE );   // + input to bandgap, enable interrupts on rising edge
  // ACME defaults to 0

}


// ------------ position  STUFF ----------------------------

 
ISR(ANALOG_COMP_vect){
  if ( IsHigh(6, PIND) ) { // alter target 
    position ++;             
  } else {
    position --; 
  }
}
































