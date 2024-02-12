#include "hiComms2.h" 
 

uint8_t  BCDLUT1[] = {0,2,4,6,8,0,2,4,6,8};  // ones place for x2
uint8_t  BCDLUT2[] = {0,0,0,0,0,1,1,1,1,1};  // carry for x2



void printNumDec32 ( uint32_t bv ) {

  uint32_t i;
  uint8_t d0, d1, d2, d3, d4, d5, d6, d7, d8, d9;  // ssh.
    
  d0 = d1 = d2 = d3 = d4 = d5 = d6 = d7 = d8 = d9 = 0;  
    
  for (i = 0x80000000; i ; i>>=1) {            
       
    d9 <<=  1;          
    d9 |= BCDLUT2[d8];
        
    d8  = BCDLUT1[d8];       
    d8 |= BCDLUT2[d7];
    
    d7 =  BCDLUT1[d7];       
    d7 |= BCDLUT2[d6];
    
    d6  = BCDLUT1[d6];       
    d6 |= BCDLUT2[d5];
    
    d5 =  BCDLUT1[d5];       
    d5 |= BCDLUT2[d4];
     
    d4 =  BCDLUT1[d4];       
    d4 |= BCDLUT2[d3];        
    
    d3  = BCDLUT1[d3];       
    d3 |= BCDLUT2[d2];
    
    d2 =  BCDLUT1[d2];       
    d2 |= BCDLUT2[d1];
     
    d1 =  BCDLUT1[d1];       
    d1 |= BCDLUT2[d0];
    
    d0 =  BCDLUT1[d0];    
    
    if (bv & i) d0 |= 1;        
    
  }  
  
  USART_Transmit( d9|0x30 );
  USART_Transmit( d8|0x30 );
  USART_Transmit( d7|0x30 );
  USART_Transmit( d6|0x30 );
  USART_Transmit( d5|0x30 );
  USART_Transmit( d4|0x30 );
  USART_Transmit( d3|0x30 );
  USART_Transmit( d2|0x30 );
  USART_Transmit( d1|0x30 );
  USART_Transmit( d0|0x30 );

}


void printSignNumDec16( int16_t bv) {
 if (bv < 0) {
   USART_Transmit( '-' );
   printNumDec16(-bv);
 } else {
   printNumDec16(bv);
 }

}


void printNumDec16( uint16_t bv ) {

  uint16_t i;
  uint8_t d0, d1, d2, d3, d4;  // ssh.
    
  d0 = d1 = d2 = d3 = d4 = 0;  
    
  for (i = 0x8000; i ; i>>=1) {            
       
    d4 <<=  1;          
    d4 |= BCDLUT2[d3];
    
    d3  = BCDLUT1[d3];       
    d3 |= BCDLUT2[d2];
    
    d2 =  BCDLUT1[d2];       
    d2 |= BCDLUT2[d1];
     
    d1 =  BCDLUT1[d1];       
    d1 |= BCDLUT2[d0];
    
    d0 =  BCDLUT1[d0];    
    
    if (bv & i) d0 |= 1;        
    
  }  
  
  USART_Transmit( d4|0x30 );
  USART_Transmit( d3|0x30 );
  USART_Transmit( d2|0x30 );
  USART_Transmit( d1|0x30 );
  USART_Transmit( d0|0x30 );

}



void printNumDec8( uint8_t bv ) {

  uint8_t i;
  uint8_t d0, d1, d2;
    
  d0 = d1 = d2 = 0;  
    
  for (i = 0x80; i ; i>>=1) {            
       
    d2 <<=  1;          
    d2 |= BCDLUT2[d1];
     
    d1 =  BCDLUT1[d1];       
    d1 |= BCDLUT2[d0];
    
    d0 =  BCDLUT1[d0];    
    
    if (bv & i) d0 |= 1;        
    
  }  
  

  USART_Transmit( d2|0x30 );
  USART_Transmit( d1|0x30 );
  USART_Transmit( d0|0x30 );

}


void printNumHex8(uint8_t i){
  uint8_t hi,lo;

 // hi=i&0xF0;               // High nibble
  hi=i>>4;
  hi+='0';
  if (hi>'9')  hi+=7;

  lo=(i&0x0F)+'0';         // Low nibble
  if (lo>'9')  lo+=7;

  USART_Transmit( hi );
  USART_Transmit( lo );
}


void printNumHex16(uint16_t v) {
  printNumHex8(v >> 8);
  printNumHex8(v & 0xFF);
}

void printNumHex32(uint32_t v) {
  printNumHex8(   v >> 24 );
  printNumHex8( ( v >> 16 ) & 0xFF);
  printNumHex8( ( v >> 8  ) & 0xFF);
  printNumHex8(   v & 0xFF);
}


void USART_printstring( char *data){
    while(*data) {
        USART_Transmit(*data);
        data++;
    }
}

