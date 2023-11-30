/*
 * Code simplified from openssl. Somehow, top gets tainted in the PREFBUF
 * function
 */
int dummy = 0;
#include "bits.h"
#include <stdlib.h>

int BN_mod_exp_mont_consttime_algorithm(BIGNUM *rr, const BIGNUM *a,
                                        const BIGNUM *p, const BIGNUM *m) {
  int i, bits, ret = 0, window, wvalue;
  int top;

  if (p->top == 0) // should not be a positive when SignatureFlowDirection < 3
    ;

  BN_is_bit_set(p, bits);
  return 1;
}
