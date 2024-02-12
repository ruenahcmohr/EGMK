/*
             CREATOR: DAn williams
                DATE:
         DESCRIPTION:
            FILENAME:    .c
             STARTED:
    OPERATING SYSTEM: Linux
1st VERSION FINISHED:



*/


#include <stdio.h>
#include <stdlib.h>
#include "vect.h"
#define MIN(a,b)    ((a) < (b) ? (a) : (b))


//global variables


/*

library for taking a list of points and abstracting the position of any point along it

input example:

x in relative forward
y in absolute

  x y
  ---
  0 0
  4 1
  4 1

result values:
 getLevel(6) = 1 
 getLevel(2) = 0.5

all interpolation to be linear >:]

*/

/*
typedef struct PointTable_s {

  double * xOffsets;
  long    xCount;
  double * yLevels;
  long    yCount; 

} PointTable_t;

*/

void initTable( PointTable_t * table ) {

  table->xOffsets = NULL;
  table->xCount   = 0;
  table->yLevels  = NULL;
  table->yCount   = 0;
 
}

int addPoint( PointTable_t * table, double x, double y ) {

 double  * tempx;
 double  * tempy;
  
 if ((tempx = realloc(table->xOffsets, sizeof(double)*((table->xCount)+1))) == NULL) {
   printf("Failure to allocate for x array\n");
   return -1;
 }
  
  if ((tempy = realloc(table->yLevels, sizeof(double)*((table->yCount)+1))) == NULL) {
    printf("Failure to allocate for y array\n");    
    table->xOffsets = realloc(table->xOffsets, sizeof(double)*(table->xCount));
    return -1;
  }
  
 // printf("x--> %ld  y-->%ld\n", table->xCount, table->yCount);
  
  table->xOffsets = tempx;
  table->yLevels  = tempy;
  
  table->xOffsets[table->xCount] = x;
  table->yLevels[table->yCount]  = y;

  table->xCount++;
  table->yCount++;  

}


double getLevel( PointTable_t * table, double x) {

// search for the value ahead and behind us
// then do simple linear interpolation >:]

  long cursor;
  double crsX;

 // printf("\nseeking: %lf\n", x);

  for ( cursor = 0, crsX = 0; 
        ((x > crsX)  &&  (cursor < (table->xCount) )); 
        crsX = table->xOffsets[cursor+1], cursor++);
         
 // printf("---> index is: %ld\n", cursor);
  
  // x == crsX  // return the value directly
  if (x == crsX) {
  // printf("x = abs\n");
   return table->yLevels[cursor];
  }
  
  // cursor == (table->xCount) // out of range for table, return last value (?)
  if (cursor == (table->xCount)) {
  // printf("past last entry\n");
    return table->yLevels[cursor-1];
  }
  
  #define x1 (table->xOffsets[cursor-1])
  #define y1 (table->yLevels[cursor-1])
  
  #define x2 (table->xOffsets[cursor])
  #define y2 (table->yLevels[cursor])
  
  #define px (x-x1)/(x2-x1)
  
 // printf("calculated %f + ((%f-%f) *     (%f-%f)/(%f-%f)) \n",y1, y2, y1, x, x1, x2, x1 );  
  return (y1+((y2-y1)*px));

}

void destroyTable( PointTable_t * table ) {
   free(table->yLevels);
   free(table->xOffsets);   
}


void dumpTable(  PointTable_t * table ) {
  long i;
  
  for (i = 0; i < table->xCount; i++) {
    printf("i:%ld, %lf, %lf\n", i, table->xOffsets[i], table->yLevels[i]);
  }
  
}

/*

int main(int argc, char** argv) {
  // local variables
  
  int retcode;
  PointTable_t data;
  
  retcode = 0;
  
  printf(" This program does line interpolation \n");
  printf(" Written by Dan Williams apr 28 2006 \n");
  
  printf("Initializing table\n");
  initTable(&data);
  
  printf("Adding points\n");
  addPoint( &data, 0, 0 );
  addPoint( &data, 4, 1 );
  addPoint( &data, 4, 1 );
  
  printf("Getting level at 0\n");
  printf("  Result is: %f\n\n", getLevel(&data, 0));
  
  printf("Getting level at 1\n");
  printf("  Result is: %f\n\n", getLevel(&data, 1));
  
  printf("Getting level at 2\n");
  printf("  Result is: %f\n\n", getLevel(&data, 2));

  printf("Getting level at 4\n");
  printf("  Result is: %f\n\n", getLevel(&data, 4));
  
  printf("Getting level at 6\n");
  printf("  Result is: %f\n\n", getLevel(&data, 6));  
  
  printf("Getting level at 9\n");
  printf("  Result is: %f\n\n", getLevel(&data, 9));  
  
  printf("deleting table\n");
  destroyTable(&data);
  
  
  return(retcode);
}
*/



