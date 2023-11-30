// Test to make sure that function arguments with gaps are handled
// Taint: n 2
// Expected: n->c branch should be tainted
// Issue: If gapped arguments is not properly handled n->d may be reported.
//        In the case of n->d tainted with n->a being unused segmentation fault
//        would occur, due to iterating past end of ConsElem map
typedef struct {
  int a;
  int b;
  int c;
  int d;
} Number;

void argument_gap(Number *n) {
  if (n->b == 2)
    ;
  if (n->c == 2) // Vulnerable due to taint.txt
    ;
  if (n->d == 2)
    ;
}
int main() {
  Number ctx;

  ctx.b = 0;
  ctx.c = 4;
  ctx.d = 3;
}
