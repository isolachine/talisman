#include <stdlib.h>

typedef struct {
  int *a;
  int b;
} S;

int bar(const S *b) { return b->a[0]; }

int foo(S *A, S *B) {
  if (bar(A))
    ;
  if (A->a[0])
    ;
  if (A->b)
    ;
  if (bar(B))
    ;
  if (B->b)
    ;
  return 0;
}
