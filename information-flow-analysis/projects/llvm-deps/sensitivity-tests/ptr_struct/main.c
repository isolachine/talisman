typedef struct bignum_st
{
  unsigned long *d;	/* Pointer to an array of 'BN_BITS2' bit chunks. */
  int top;	/* Index of last used d +1. */
  /* The next are internal book keeping for bn_expand. */
  int dmax;	/* Size of the d array. */
  int neg;	/* one if the number is negative */
  int flags;
} BIGNUM;

int testfunc(const BIGNUM * a)
#if 1
{
  if (a->top == 2) // tainted - loads from c
    ;

  if ((a->d[0] & 1) == 1) // tainted - loads from c
    ;

  return a->top;
}
#endif

int test2(const BIGNUM * b);

int main(void)
{
  BIGNUM c; // tainted in taint.txt

  if(testfunc(&c) == 0) // result of testfunc 2 is defined and tainted
    ;

  if (test2(&c) == 0) // result of test2 is undefined and thus tainted
    ;
}
