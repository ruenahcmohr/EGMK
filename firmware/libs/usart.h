// AVR306: Using the AVR UART in C
// Routines for polled USART
// Last modified: 02-06-21
// Modified by: AR

/* Includes */
#include <avr/io.h>
#include <stdio.h>
#include <stdarg.h>

#define HAVEDATA (UCSR0A & (1<<RXC0))
#define SENDING  (!(UCSR0A & (1<<UDRE0)))

/* Prototypes */

void USART_Init( unsigned int baudrate );
unsigned char USART_Receive( void );
void USART_Transmit( unsigned char data );

/*
void USART_printint(unsigned int data) ;
void USART_printhex(uint8_t i);
void USART_printstring(char *data);
void USART_print5(unsigned int x) ;
*/
