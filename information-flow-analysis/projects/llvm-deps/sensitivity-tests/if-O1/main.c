#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define PRINT printf(" ")

int function(int key, int key2) {
  int u, m;
  srand(time(NULL));

  u = key;
  m = key2 + rand();

  for (u = 1; u < m; u++)
    PRINT;

  return u;
}
