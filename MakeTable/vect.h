


typedef struct PointTable_s {

  double * xOffsets;
  long    xCount;
  double * yLevels;
  long    yCount; 

} PointTable_t;

void  initTable   ( PointTable_t * table ) ;
int   addPoint    ( PointTable_t * table, double x, double y ) ;
double getLevel    ( PointTable_t * table, double x);
void  destroyTable( PointTable_t * table );
void dumpTable(  PointTable_t * table ) ;

