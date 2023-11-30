#include <stdio.h>

typedef struct {
  int x;
  int y;
  int z;
} Context;

int main(void) {
  Context c, d;
  int key = 0; // this is tainted

  c.x = key;
  c.y = 1;
  c.z = 1;

  // d is tainted
  d.x = 0;
  d.y = 0;
  d.z = 0;

  if (key) {
    d.y++;

    if (c.x == 0) {
      d.x++;
    }

    if (c.z == 0) {
      d.x++;
    }
  }

  return 0;
}
