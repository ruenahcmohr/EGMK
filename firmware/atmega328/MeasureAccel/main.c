/******************************************************************************
Title:    EGMK DC stepped servo
Author:   rue_mohr
Date:     Aug 19 2005
Software: AVR-GCC 3.3
Hardware: atmega32


III N   N  PPPP  RRRR   OOO   GGG  RRRR  EEEE  SSS   SSS
 I  NN  N  P   P R   R O   O G     R   R E    S     S
 I  N N N  PPPP  RRRR  O   O G  GG RRRR  EEE   SSS   SSS
 I  N  NN  P     R R   O   O G   G R R   E        S     S
III N   N  P     R  RR  OOO   GGG  R  RR EEEE  SSS   SSS

L298 motor driver
  PB4 to L293 pin 1
  PB2 to L293 pin 7
  inverted PB3 to L293 pin 2



pins:

  on pro mini:

0   rxd    rxd
1   txd    txd

2   pd2   encoder A
3   pd3   encoder B
4   pd4
5   pd5
6   pd6  DIR
7   pd7  STEP

8   pb0
9   pb1
10  pb2  motor driver
11  pb3  motor driver
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

 This applies timed pulses to the motor and records the resulting velocity and movement at the end of the 
 drive pulse.


*******************************************************************************/

/****************************| INCLUDGEABLES |********************************/

#include <avr/io.h>
#include <avr/interrupt.h>
// #include <avr/signal.h>
#include "usart.h"
#include "avrcommon.h"
#include "nopDelay.h"
#include "hiComms2.h"
#include "localsys.h"

/*****************************| DEFINIATIONS |********************************/

#define DeadZone 18

/*****************************| VARIABLES |********************************/


volatile int position; // note that this is signed
volatile char direction;
volatile unsigned int moveTicks;
volatile char flag;
volatile int wait = 5; // 10 * .1ms = 1ms
volatile char printFlag;
volatile int count;
volatile int encoderCount;
int minCorrectionDistance = 0;
int error;
int v;
char buf[8];
int stopPos;

#define ERROR 0
signed char offsets[] = {
    /* 0000 */ 0,
    /* 0001 */ +1,
    /* 0010 */ -1,
    /* 0011 */ ERROR,
    /* 0100 */ -1,
    /* 0101 */ 0,
    /* 0110 */ ERROR,
    /* 0111 */ +1,
    /* 1000 */ +1,
    /* 1001 */ ERROR,
    /* 1010 */ 0,
    /* 1011 */ -1,
    /* 1100 */ ERROR,
    /* 1101 */ -1,
    /* 1110 */ +1,
    /* 1111 */ 0};

/************************| FUNCTION DECLARATIONS |*************************/

void setupEncoder(void);
void updatePos(void);
void speed_init(void);
void setupStepDir(void);
void pulseTimerInit(void);

/****************************| CODE... |***********************************/


int main(void)
{


  DDRB = (INPUT << PB0 | INPUT << PB1 | OUTPUT << PB2 | OUTPUT << PB3 | INPUT << PB4 | INPUT << PB5 | INPUT << PB6 | INPUT << PB7);
  DDRC = (INPUT << PC0 | INPUT << PC1 | INPUT << PC2 | INPUT << PC3 | INPUT << PC4 | INPUT << PC5 | INPUT << PC6);
  DDRD = (INPUT << PD0 | INPUT << PD1 | INPUT << PD2 | INPUT << PD3 | INPUT << PD4 | INPUT << PD5 | INPUT << PD6 | INPUT << PD7);

  PORTC = 0xFF; // turn on pullups.

  USART_Init(16); // 57600
  USART_printstring("...\n");

  speed_init();
  setupEncoder(); // encoder inputs
  setupStepDir(); // step-direction inputs
  pulseTimerInit();
  MotorOff();
  Delay(724637); // 2 second startup delay
  Delay(724637);
  USART_printstring("Position (lines),");
  USART_printstring("Pulse (0.1ms)");
  USART_printstring("\n");

  sei();

  MotorForward();
  while (1)
  {
    if (printFlag)
    {
      cli();
      MotorOff();
   
      printNumDec16(encoderCount);
      USART_printstring(",");
      printNumDec16(wait);//time in ms
      USART_printstring("\n");
      Delay(724637);
      encoderCount = 0;
      TCNT1 = 0;
      TCNT0 = 0;
      MotorForward();
      printFlag = 0;
      sei();
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

//-------------------------- PULSE TIMER --------------------------//

// level 1 interrupt, higher priority than encoder and step/dir
void pulseTimerInit()
{
  // 16mhz / 8 = 2mhz /200 = 10khz = 100us = .1ms per trigger
  TCNT0 = 0;               // clear counter
  TCCR0B |= (1 << CS01);   // prescaler 8
  OCR0A = 20; //0xC8;            // compare value of 200
  TIMSK0 |= (1 << OCIE0A); // enable compare match interrupt
}

ISR(TIMER0_COMPA_vect)
{ // compare match interrupt
  
  if (count >= wait)
  {
   
    wait++; // add 1 ms to the wait time
    count = 0;
    printFlag = 1;
  }
  else{
  count++;
  }
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

void updatePos()
{
  static unsigned char oldstate;
  signed char t;

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
  encoderCount++;
  if (direction == t)
  { // if the direction is the same
    moveTicks = TCNT1;
  }
  else
  {
    moveTicks = 30000; // "0" if we changed direction and dont have a new reading yet.
    t = 0;             // set direction to zero. (average, we just turned around)
  }

  TCNT1 = 0;
  direction = t;
  flag++;
}

//------------------------- STEP,DIR INPUT STUFF ------------------------------------

void setupStepDir()
{

  /*
    ACME   of ADCSRB = 0; // negative input is AIN1
    SCIS0  of ACSR   = 1; // interrupt on comparator rising edge
    SCIS1  of ACSR   = 1; // "
    ACBG   of ACSR   = 1; // use the badgap for + input
    ACIE   of ACSR   = 1; // enable comparator interrupt

  */

  ACSR = (1 << ACIS1) | (1 << ACBG) | (1 << ACIE); // + input to bandgap, enable interrupts on rising edge
  // ACME defaults to 0
}

ISR(ANALOG_COMP_vect)
{
  if (IsHigh(6, PIND))
  { // alter target
    position -= step;
  }
  else
  {
    position += step;
  }

  flag++;
}
