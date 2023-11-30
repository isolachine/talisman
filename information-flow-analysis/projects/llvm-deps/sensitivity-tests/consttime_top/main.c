#include "v4_4_mont_consttime_lib/bn_lib.c"
#include "internal/cryptlib.h"
#include "internal/constant_time_locl.h"
#include "bn_lcl.h"
int dummy = 0;
#include <stdlib.h>
#include "rsaz_exp.h"

static int MOD_EXP_CTIME_COPY_TO_PREBUF(const BIGNUM *b, int top,
                                        unsigned char *buf, int idx,
                                        int window)
{
    int i, j;
    int width = 1 << window;
    BN_ULONG *table = (BN_ULONG *)buf;
	
	if (top > b->top)
        top = b->top;           /* this works because 'buf' is explicitly
                                 * zeroed */
    i = 0, j = idx; if(top)dummy++; {/*loop*/ //for (i = 0, j = idx; i < top; i++, j += width) {
        table[j] = b->d[i];
    }

    return 1;
}

int BN_mod_exp_mont_consttime_algorithm(BIGNUM *rr, const BIGNUM *a, const BIGNUM *p,
                                        const BIGNUM *m, BN_CTX *ctx, BN_MONT_CTX *in_mont,BIGNUM *pub_BIGNUM0,BIGNUM *pub_BIGNUM1,BIGNUM *pub_BIGNUM2,int pub_BRANCH0)
{
    int i, bits, ret = 0, window, wvalue;
    int top; unsigned char aa = 'a'; unsigned char bb = 'b';
    BN_MONT_CTX *mont = NULL;
    unsigned char *chr0 = &aa;unsigned char *chr1 = &bb;
    int numPowers;
    unsigned char *powerbufFree = NULL;
    int powerbufLen = 0;
    unsigned char *powerbuf = NULL;
    BIGNUM tmp, am;

    top = m->top;
	
    /* lay down tmp and am right after powers table */
    tmp.d = (BN_ULONG *)(powerbuf + sizeof(m->d[0]) * top * numPowers);
    am.d = tmp.d + top;
    tmp.top = am.top = 0;
    tmp.dmax = am.dmax = top;
    tmp.neg = am.neg = 0;
    tmp.flags = am.flags = BN_FLG_STATIC_DATA;

        tmp.d[0] = (0 - m->d[0]) & BN_MASK2;
//        i = 1; if(i + top)dummy++;/*loop*/ //for (i = 1; i < top; i++)
//            tmp.d[i] = (~m->d[i]) & BN_MASK2;
//        tmp.top = top;

        MOD_EXP_CTIME_COPY_TO_PREBUF(&tmp, top, powerbuf, 0, window);

	

err:dummy++;
    return (ret);
}
