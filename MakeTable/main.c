#include <stdio.h>
#include "vect.h"


#define divider 128.0

int main(int argc, char** argv) {
  // local variables
  
  int retcode;
  PointTable_t data;
  int i;
  
  retcode = 0;
  
  printf("Initializing table\n");
  initTable(&data);
  
  printf("Adding points - speed, stopping-distance\n");
  addPoint( &data,   0,              64 );
  addPoint( &data,   5999.0/divider, 64 );
  addPoint( &data,   6000.0/divider, 56 );
  addPoint( &data,   6500.0/divider, 48 );
  addPoint( &data,   7000.0/divider, 42 );
  addPoint( &data,   8000.0/divider, 32 );
  addPoint( &data,   9000.0/divider, 26 );
  addPoint( &data,  10000.0/divider, 22 );
  addPoint( &data,  11000.0/divider, 18 );
  addPoint( &data,  13000.0/divider, 12 );
  addPoint( &data,  16000.0/divider, 8  );
  addPoint( &data,  19000.0/divider, 6 );
  addPoint( &data,  22000.0/divider, 4 );
  addPoint( &data,  27000.0/divider, 2 );
 
  
  //dumpTable( &data );
  
  printf("uint8_t VLUT[256] = {\n");
  
  for( i = 0; i < 256; i++ ) {
    printf("%03d,", (int)getLevel(&data, i ));    
  }
  
  printf("};\n\n");
 
  for( i = 0; i < 256; i++ ) {
    printf("%03d,%03f \n", (int)getLevel(&data, i ), i*divider);    
  } 
 
 
  destroyTable(&data);
  
  
  return(retcode);
}



/*

uint8_t VLUT[256] = {
  250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,250,
  250,250,250,250,250,250,250,250,250,250,250,250,242,226,210,198,
  186,176,168,160,152,144,138,134,130,126,122,118,114,110,106,102,
  99,97,95,92,89,86,83,80,78,76,74,72,70,68,67,66,
  64,63,62,60,59,58,56,55,54,52,51,50,49,48,48,47,
  47,46,45,45,44,44,43,42,42,41,41,40,39,39,38,38,
  37,36,36,35,35,34,34,33,33,33,32,32,31,31,31,30,
  30,29,29,29,28,28,27,27,27,26,26,25,25,25,24,24,
  24,24,24,23,23,23,23,23,22,22,22,22,22,21,21,21,
  21,21,20,20,20,20,20,19,19,19,19,19,18,18,18,18,
  18,17,17,17,17,17,16,16,16,16,16,15,15,15,15,15,
  14,14,13,13,13,12,12,11,11,11,10,10,9,9,9,8,
  8,7,7,7,6,6,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
  5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
  5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5
};


*/
