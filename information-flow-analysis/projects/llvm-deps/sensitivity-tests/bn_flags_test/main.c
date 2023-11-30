#include <stdlib.h>

#define BN_ULONG        unsigned long
#define BN_FLG_CONSTTIME        0x04
struct bignum_st {
  BN_ULONG *d;                /* Pointer to an array of 'BN_BITS2' bit
                               * chunks. */
  int top;                    /* Index of last used d +1. */
  /* The next are internal book keeping for bn_expand. */
  int dmax;                   /* Size of the d array. */
  int neg;                    /* one if the number is negative */
  int flags;
};

typedef struct bignum_st BIGNUM;

int BN_get_flags(const BIGNUM *b, int n)
{
  return b->flags & n;
}


#if 1
int main() {
  const BIGNUM * p = malloc(sizeof(*p));

  if(p->flags == 2)
    ;

  if(p->dmax == 0)
    ;

  if(BN_get_flags(p, BN_FLG_CONSTTIME)!= 0)
    ;

  free((void*) p);
  p = NULL;


}

#endif
