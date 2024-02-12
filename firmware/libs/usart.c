// AVR306: Using the AVR UART in C
// Routines for polled USART
// Last modified: 02-06-21
// Modified by: AR

/* Includes */
#include "usart.h"

/* Initialize UART */
void USART_Init( unsigned int baudrate ) {
	/* Set the baud rate */
	UBRR0H = (unsigned char) (baudrate>>8);                  
	UBRR0L = (unsigned char) baudrate;
	
	/* Enable UART receiver and transmitter */
	UCSR0B = ( ( 1 << RXEN0 ) | ( 1 << TXEN0 ) ); 
	
	/* Set frame format: 8 data 2stop */
	//UCSRC = (1<<USBS)|(1<<UCSZ1)|(1<<UCSZ0);              //For devices with Extended IO
	//UCSRC = (1<<URSEL)|(1<<USBS)|(1<<UCSZ1)|(1<<UCSZ0);   //For devices without Extended IO
}


// Read and write functions 
unsigned char USART_Receive( void ) {
	// Wait for incomming data 
	while ( !HAVEDATA ) ;			                
	return UDR0;
}


void USART_Transmit( unsigned char data ) {
	// Wait for empty transmit buffer 
	while ( SENDING );
	// Start transmittion 
	UDR0 = data; 			        
}

/*
void USART_printhex(uint8_t i) {
    uint8_t hi,lo;

    hi=i&0xF0;               // High nibble
    hi=hi>>4;
    hi=hi+'0';
    if (hi>'9')
        hi=hi+7;

    lo=(i&0x0F)+'0';         // Low nibble
    if (lo>'9')
        lo=lo+7;

USART_Transmit( hi );
USART_Transmit( lo );
}

void USART_printint(unsigned int data) {
   unsigned int  p;
   for ( p = 10000; p > 0; p /= 10) {
      USART_Transmit('0' + (data / p));
      data %= p;
   }
}

void USART_print5(unsigned int x) {
  unsigned int y;

    y=x/10000;  USART_Transmit(y+0x30);x-=(y*10000);
    y=x/1000;  USART_Transmit(y+0x30);x-=(y*1000);
    y=x/100;  USART_Transmit(y+0x30);x-=(y*100);
    y=x/10; USART_Transmit (y+0x30);x-=(y*10);
    USART_Transmit (x+0x30);
}

void USART_printstring( char *data) {
    while(*data) {
        USART_Transmit(*data);
        data++;
    }

}
*/
