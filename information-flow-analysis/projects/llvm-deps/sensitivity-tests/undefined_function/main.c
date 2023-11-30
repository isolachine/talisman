#include <stdio.h>

int flow_between_args(char* a, char b);

int defined_flow(char* c, char d) {
  *c = d;
  return 0;
}

int main() {
  char p;
  char q = 2;
  int r = defined_flow(&p, q);

  char s = 0;
  char t = 0;
  int u = defined_flow(&s, t);

  char x;
  char y;
  int z = flow_between_args(&x, y);


  char a;
  char b;
  int c = flow_between_args(&a, b);

  if (p == 0) // should be untainted, but the AbsLoc for c is tainted (from t) and flows to d
    ;
  if (q == 0) // untainted
    ;
  if (r == 0) // untainted return 0 shows no information
    ;

  if (s == 0)  // tainted if has definition *c = d in defined function
    ;
  if (t == 0) // tainted by taint.txt
    ;
  if (u == 0) // untainted because no arguments are tainted
    ;

  if (x == 0) // tainted from y because of undefined function
    ;
  if (y == 1) // tainted by taint.txt
    ;
  if (z == 1) // tainted from y because of undefined function
    ;


  if (a == 0) // Tainted by taint.txt
    ;
  if (b == 0) // Untainted even with undefined signature due to copy
    ;
  if (c == 0) // Tainted because of undefined signature
    ;
  return 0;
}
