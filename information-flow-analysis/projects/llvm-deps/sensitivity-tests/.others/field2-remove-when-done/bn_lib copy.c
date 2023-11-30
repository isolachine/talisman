#include "bn_lcl.h"
#include "internal/cryptlib.h"
#include <assert.h>
#include <limits.h>
#include <openssl/opensslconf.h>

static void bn_free_d(BIGNUM *a) {
  if (BN_get_flags(a, BN_FLG_SECURE))
    OPENSSL_secure_free(a->d);
  else
    OPENSSL_free(a->d);
}

void BN_clear_free(BIGNUM *a) {
  int i;

  if (a == NULL)
    return;
  bn_check_top(a);
  if (a->d != NULL) {
    OPENSSL_cleanse(a->d, a->dmax * sizeof(a->d[0]));
    if (!BN_get_flags(a, BN_FLG_STATIC_DATA))
      bn_free_d(a);
  }
  i = BN_get_flags(a, BN_FLG_MALLOCED);
  OPENSSL_cleanse(a, sizeof(*a));
  if (i)
    OPENSSL_free(a);
}

static BN_ULONG *bn_expand_internal(const BIGNUM *b, int words) {
  BN_ULONG *A, *a = NULL;
  const BN_ULONG *B;
  int i;

  B = b->d;
  if (B != NULL) {
    for (i = b->top >> 2; i > 0; i--, A += 4, B += 4) {
    }
  }

  return (a);
}

BIGNUM *bn_expand2(BIGNUM *b, int words) {
  bn_check_top(b);

  if (words > b->dmax) {
    BN_ULONG *a = bn_expand_internal(b, words);
    if (!a)
      return NULL;
    if (b->d) {
      OPENSSL_cleanse(b->d, b->dmax * sizeof(b->d[0]));
      bn_free_d(b);
    }
    b->d = a;
    b->dmax = words;
  }

  bn_check_top(b);
  return b;
}

int BN_get_flags(const BIGNUM *b, int n) { return b->flags & n; }
