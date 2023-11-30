/*
 * Copyright 1995-2016 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */

#include "internal/cryptlib.h"
#include "internal/constant_time_locl.h"
#include "bn_lcl.h"

#include <stdlib.h>
#ifdef _WIN32
# include <malloc.h>
# ifndef alloca
#  define alloca _alloca
# endif
#elif defined(__GNUC__)
# ifndef alloca
#  define alloca(s) __builtin_alloca((s))
# endif
#elif defined(__sun)
# include <alloca.h>
#endif

#include "rsaz_exp.h"

#undef SPARC_T4_MONT
#if defined(OPENSSL_BN_ASM_MONT) && (defined(__sparc__) || defined(__sparc))
# include "sparc_arch.h"
extern unsigned int OPENSSL_sparcv9cap_P[];
# define SPARC_T4_MONT
#endif

/* maximum precomputation table size for *variable* sliding windows */
#define TABLE_SIZE      32

/* this one works - simple but works */
int BN_mod_exp2(int *r, const int *a, const int *p, const int *m,
               BN_CTX *ctx)
{
    int ret;

    /*
     * I have finally been able to take out this pre-condition of the top bit
     * being set.  It was caused by an error in BN_div with negatives.  There
     * was also another problem when for a^b%m a >= m.  eay 07-May-97
     */
    /* if ((m->d[m->top-1]&BN_TBIT) && BN_is_odd(m)) */

    if(m[0] == 3)
      ;

    return (ret);
}

