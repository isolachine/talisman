#include <stdio.h>

int undefined_func(char* a, char b);

int GLOBAL = 0;

int main() {
  char* a, b;
  int c;

  char* x;
  int y;

  c = undefined_func(a, b);
  if (a)
    ;
  if (b)
    ;
  if (c)
    ;

  y = undefined_func(GLOBAL, x);
  if (x)
    ;
  if (y)
    ;

  return 0;
}
