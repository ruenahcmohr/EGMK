#include <stdio.h>
#include "vect.h"


int main(int argc, char** argv) {
  // local variables
  
  int retcode;
  PointTable_t data;
  int i;
  
  retcode = 0;
  
  printf("Initializing table\n");
  initTable(&data);
  
  printf("Adding points\n");
  addPoint( &data,  0, 250 );
  addPoint( &data,  220.0/8.0, 250 );
  addPoint( &data,  240.0/8.0, 210 );
  addPoint( &data,  260.0/8.0, 180 );
  addPoint( &data,  280.0/8.0, 160 );
  addPoint( &data,  300.0/8.0, 140 );
  addPoint( &data,  320.0/8.0, 130 );
  addPoint( &data,  340.0/8.0, 120 );
  addPoint( &data,  360.0/8.0, 110 );
  addPoint( &data,  380.0/8.0, 100 );
  addPoint( &data,  400.0/8.0, 95  );
  addPoint( &data,  440.0/8.0, 80 );
  addPoint( &data,  480.0/8.0, 70 );
  addPoint( &data,  540.0/8.0, 60 );
  addPoint( &data,  600.0/8.0, 50 );
  addPoint( &data,  800.0/8.0, 35 );
  addPoint( &data, 1000.0/8.0, 25 );
  addPoint( &data, 1200.0/8.0, 20 );
  addPoint( &data, 1400.0/8.0, 15 );
  addPoint( &data, 1600.0/8.0, 5 );
  
  //dumpTable( &data );
  
  printf("uint8_t VLUT[256] = {\n");
  
  for( i = 0; i < 256; i++ ) {
    printf("%03d,", (int)getLevel(&data, i ));    
  }
  
  printf("};\n");
 
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
