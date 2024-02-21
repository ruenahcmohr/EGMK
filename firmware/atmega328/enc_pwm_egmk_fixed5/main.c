
/******************************************************************************
Title:    EGMK DC stepped servo
Author:   rue_mohr
Date:     Aug 19 2005
Software: AVR-GCC 3.3
Hardware: atmega32


pins:

  on pro mini:

0   rxd  rxd
1   txd  txd

2   pd2  encoder A
3   pd3  encoder B
4   pd4
5   pd5
6   pd6  control DIR
7   pd7  control STEP

8   pb0
9   pb1
10  pb2  motor driver DIR
11  pb3  motor driver PWM
12  pb4
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

 EGMK stepper, this performs the step, then leaves the motor idle with no post-corrections
 state based.


*******************************************************************************/

/****************************| INCLUDGEABLES |********************************/

#include <avr/io.h>
#include <avr/interrupt.h>
#include "usart.h"
#include "avrcommon.h"
#include "nopDelay.h"
#include "hiComms2.h"
#include <stdint.h>

/*****************************| DEFINIATIONS |********************************/

// power     is PB4
// direction is PB2

 // Rues LM18200 setup
 #define  MotorForward()  SetBit(2, PORTB) ;    SetBit(3, PORTB)    
 #define  MotorReverse()  ClearBit(2, PORTB);   SetBit(3, PORTB)  
 #define  MotorOff()      ClearBit(2, PORTB) ;  ClearBit(3, PORTB)  


// Gear Reduction Ratio obfuscated_d: 1/362, Each Loop Output Pulses: 12PPR 362*12=4344PPR;

// step size of 1260 means Rue gets 36 degrees per step

/*****************************| VARIABLES |********************************/

// #define step 1260
// #define step 630
// #define step 315
// #define  step  144
// #define  step  72
// #define  step  36
#define step 63

volatile int          position; // note that this is signed
volatile char         direction;
volatile unsigned int moveTicks;
volatile uint8_t      flag;


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

void setupEncoder(void);
void updatePos(void);
void speed_init(void);
void setupStepDir(void);



/****************************| CODE... |***********************************/

 uint8_t VLUT[256] = {
     135,135,135,135,135,135,135,135,135,135,135,135,135,135,135,135,135,
     135,119,103,98,93,87,82,78,76,73,70,68,65,63,60,58,55,52,50,47,45,42,
     40,39,38,38,37,36,36,35,34,34,33,33,32,31,31,30,29,29,28,27,27,26,25,
     25,24,24,23,22,22,21,20,20,19,19,19,18,18,18,17,17,17,16,16,16,15,15,
     15,14,14,14,14,14,14,14,14,13,13,13,13,13,13,13,13,12,12,12,12,12,12,
     12,12,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,11,10,10,
     10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,9,9,9,9,9,9,9,9,
     9,9,9,9,9,9,9,9,9,9,9,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
     8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
     8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,0
};


// self-learn data... (shift 4)
/*
uint8_t VLUT[256] = {
  128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,
  128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,128,
  128,128,42,41,38,38,36,37,35,34,35,33,32,32,29,30,31,31,26,31,29,29,28,23,27,26,27,
  24,24,25,26,23,23,24,24,22,21,22,22,24,20,21,19,21,20,15,18,19,19,19,17,20,17,17,14,
  17,16,13,16,16,16,16,15,15,15,14,16,14,13,15,15,14,13,15,13,12,13,12,15,14,13,12,12,
  12,11,12,12,12,17,11,11,12,10,0,11,10,11,16,11,0,3,1,11,0,1,0,1,1,0,1,1,1,0,1,0,1,0,
  1,1,0,1,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,1,0,0,1,1,1,0,0,0,0,0,1,0,0,0,0,0,0,
  0,0,0,3,1,0,0,0,1,0,2,1,2,0,5,0,2,0,3,0,0,1,0,8,0,1,0,2,2,0,0,3,4,7,0,4,1,0,1,0,0,2,
  8,0,0,0,1,7,7,7,3,2,6,7,1
};
*/

int main(void) {

  int8_t motorDrive = 0;
  
  uint8_t driveState = 0;
  unsigned int v;
  int p;  
  int stopPos;
  

  DDRB = (INPUT << PB0 | INPUT << PB1 | OUTPUT << PB2 | OUTPUT << PB3 | INPUT << PB4 | INPUT << PB5 | INPUT << PB6 | INPUT << PB7);
  DDRC = (INPUT << PC0 | INPUT << PC1 | INPUT << PC2  | INPUT << PC3  | INPUT << PC4 | INPUT << PC5 | INPUT << PC6);
  DDRD = (INPUT << PD0 | INPUT << PD1 | INPUT << PD2  | INPUT << PD3  | INPUT << PD4 | INPUT << PD5 | INPUT << PD6 | INPUT << PD7);

  USART_Init(16); // 57600
  USART_printstring("...\n");

  speed_init();
  setupEncoder(); // encoder inputs
  setupStepDir(); // step-direction inputs
  
  // show me your directions work on the motor...
  MotorForward();
  Delay(724637); 
  MotorReverse();
  Delay(724637);
  MotorOff();
  Delay(724637);
  Delay(724637);
  
  
  USART_printstring(">>>\n");

  sei();
  
  driveState = 0;
  flag       = 0;

  while(1) {
         
    while (flag == 0);    // wait for system status change    
    if ((flag & 2) != 0)  driveState = 0;  
    flag = 0;            	    
		    
    // quick and grab data       
    cli();    
      v = moveTicks; 
      p = position;
    sei();
    
    // build table index, make educated guess of stop position.
    
    v >>= 4;  v = (v > 255) ? 255 : v ;
    stopPos = p + ((direction > 0) ? (VLUT[v]) : (-VLUT[v]));
    
    #define DEADZONE 2
    
    /*
     state transitions like this.
      
     0 -> 6
     0 -> 1 -> 2 -> 4 -> 6     
     0 -> 1 -> 3 -> 5 -> 6
    */
    
    
    if (driveState == 0) { // new step precheck
      if (ABS(p) > DEADZONE) {
        driveState = 1; // start accel
      } else {
        driveState = 6; // maintain / idle 		 // if were moving faster than X, should we go to state 3 or 4 instead?        						                                         //   maybe this would mean the incomming velocity is too high? 
      }
    }
    
      if (driveState == 1) { // accel    
      if (p < 0) { // we already know its outside the deadzone
        motorDrive = 1; // forward, we need positive encoder counts to fix this
        driveState = 2;
      } else {
        motorDrive = -1; // reverse, we need negative encoder counts to fix this.
        driveState = 3;
      }
    }
      
    if (driveState == 2) { // waiting during accel    
      if  (stopPos > 0) driveState = 4;  // - --> + done detection   
            
    } else if (driveState == 3) { // waiting during accel          
      if (stopPos < 0)  driveState = 5;  // - <-- + done detection         
        
    }
    
            
    if (driveState == 4) { // decel rolling forward  
      if (direction == 0) { // are we done?
         motorDrive = 0; // coast
         driveState = 6; // maintain/idle
      } else if (stopPos < 0) { // coast if it looks were going to stop short
         motorDrive = 0; // coast         
      } else {
         motorDrive = -1; // reverse      
      }
      
    } else if (driveState == 5) { // decel rolling reverse
      if (direction == 1) { // are we done?
         motorDrive = 0; // coast   
         driveState = 6; // maintain/idle
      } else if (stopPos > 0) { // coast if it looks were going to stop short
         motorDrive = 0; // coast       
      } else {
         motorDrive = 1; // forward 
      }            
    
    }
    
    if (driveState == 6) {
      motorDrive = 0; // coast  
    }
    
    // incase the state machine changes its mind a few times, motorDrive is the result drive siganl
    if (motorDrive == 0) {
      MotorOff(); 
    } else if (motorDrive > 0) {
       MotorForward();
    } else {
       MotorReverse(); 
    }
   
  }

}


//------------------------| FUNCTIONS |------------------------





//-------------------------- QUADRATURE ENCODER --------------------------

ISR(TIMER1_OVF_vect)
{
  moveTicks = 60000;
  direction = 0;
}

void speed_init()
{
  TCNT1 = 0;
  TCCR1B = 0 << CS12 | /* prescale by 1*/
           0 << CS11 |
           1 << CS10;
}

// ------------ position  STUFF ----------------------------

void setupEncoder()
{
  position = 0;                        // clear position
  EICRA = (1 << ISC10) | (1 << ISC00); // we need to set up int0 and int1 to trigger interrupts on both edges
  EIMSK = (1 << INT0) | (1 << INT1);   // then enable them.
}

// SIGNAL (SIG_INTERRUPT0) {  // fix this signal name!!! INT0
ISR(INT0_vect)
{
  updatePos();
}

// SIGNAL (SIG_INTERRUPT1) { // fix this signal name!!! INT1
ISR(INT1_vect)
{
  updatePos();
}

void updatePos() {
  static unsigned char oldstate;
  int8_t t;

  /*      get the bits        PIND     = XXXXiiXX
          clear space       & 0x0C     = 0000ii00
          or in old status  | oldstate = XX00iijj
          clean up          & 0x0F     = 0000iijj  */

  oldstate = oldstate | (PIND & 0x0C); // Update Oldstate
  t = offsets[oldstate];
  oldstate = oldstate >> 2;

  if (t == 0)
    return; // ignore glitch pulses

  position += t; // Update Position

  if ((direction == t) || (direction == 0)) { // if the direction is the same as last time *** important correction here ****
    moveTicks = TCNT1;
  } else  {
    moveTicks = 60000; // "0" if we changed direction and dont have a new reading yet.
    t = 0;             // set direction to zero. (average, we just turned around)
  }

  TCNT1     = 0;
  direction = t;
  flag |= 1;
}

//------------------------- STEP,DIR INPUT STUFF ------------------------------------

void setupStepDir() {

/*
  ACME   of ADCSRB = 0; // negative input is AIN1
  SCIS0  of ACSR   = 0; // interrupt on comparator rising edge 
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
    position += step;             
  } else {
    position -= step; 
  }
  flag |= 2;
  

  
}
