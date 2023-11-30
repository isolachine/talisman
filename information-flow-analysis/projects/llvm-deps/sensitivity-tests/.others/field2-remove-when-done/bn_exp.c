/*
 * Copyright 1995-2016 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include "bn_lcl.h"
#include "internal/constant_time_locl.h"
#include "internal/cryptlib.h"

#include <stdlib.h>
#ifdef _WIN32
#include <malloc.h>
#ifndef alloca
#define alloca _alloca
#endif
#elif defined(__GNUC__)
#ifndef alloca
#define alloca(s) __builtin_alloca((s))
#endif
#elif defined(__sun)
#include <alloca.h>
#endif

#include "rsaz_exp.h"

#undef SPARC_T4_MONT
#if defined(OPENSSL_BN_ASM_MONT) && (defined(__sparc__) || defined(__sparc))
#include "sparc_arch.h"
extern unsigned int OPENSSL_sparcv9cap_P[];
#define SPARC_T4_MONT
#endif

/* maximum precomputation table size for *variable* sliding windows */
#define TABLE_SIZE 32

int BN_mod_exp_recp(BIGNUM *r, const BIGNUM *a, const BIGNUM *p,
                    const BIGNUM *m, BN_CTX *ctx) {
  int i, j, bits, ret = 0, wstart, wend, window, wvalue;
  int start = 1;
  BIGNUM *aa;
  /* Table of variables obtained from 'ctx' */
  BIGNUM *val[TABLE_SIZE];
  BN_RECP_CTX recp;

  if (BN_get_flags(p, BN_FLG_CONSTTIME) != 0 ||
      BN_get_flags(a, BN_FLG_CONSTTIME) != 0 ||
      BN_get_flags(m, BN_FLG_CONSTTIME) != 0) {
    /* BN_FLG_CONSTTIME only supported by BN_mod_exp_mont() */
    BNerr(BN_F_BN_MOD_EXP_RECP, ERR_R_SHOULD_NOT_HAVE_BEEN_CALLED);
    return 0;
  }

  bits = BN_num_bits(p);
  if (bits == 0) {
    /* x**0 mod 1 is still zero. */
    if (BN_is_one(m)) {
      ret = 1;
      BN_zero(r);
    } else {
      ret = BN_one(r);
    }
  }

  BN_CTX_start(ctx);
  aa = BN_CTX_get(ctx);
  val[0] = BN_CTX_get(ctx);
  if (!aa || !val[0])
    goto err;

  BN_RECP_CTX_init(&recp);
  if (m->neg) {
    /* ignore sign of 'm' */
    if (!BN_copy(aa, m))
      goto err;
    aa->neg = 0;
    if (BN_RECP_CTX_set(&recp, aa, ctx) <= 0)
      goto err;
  } else {
    if (BN_RECP_CTX_set(&recp, m, ctx) <= 0)
      goto err;
  }

  if (!BN_nnmod(val[0], a, m, ctx))
    goto err; /* 1 */
  if (BN_is_zero(val[0])) {
    BN_zero(r);
    ret = 1;
    goto err;
  }

  if (p->flags) {
  }
  if (p->d) {
  }
  if (p->d[0]) {
  }
err:
  return 0;
}
