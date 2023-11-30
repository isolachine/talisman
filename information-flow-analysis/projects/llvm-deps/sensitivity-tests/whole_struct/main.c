#include <stdio.h>


typedef struct {
  int x;
  int y;
  int z;
} Context;


int main(void)
{
  Context c, d;
  int key = 0; // this is tainted

  c.x = key;
  c.y = 1;
  c.z = 1;

  // d is tainted
  d.x = 0;
  d.y = 0;
  d.z = 0;


  if(c.x == 1) // Reported - Probably shouldn't be though
    ;
  if(c.y == 1)
    ;
  if(c.z == 1)
    ;
  if(d.x == 1) // Reported
    ;
  if(d.y == 1) // Reported
    ;
  if(d.z == 1) // Reported
    ;

  return 0;
}
