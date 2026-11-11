/* SPDX-License-Identifier: GPL-2.0-only */
#include "lengths.h"
#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main(void) {
    char *input=malloc(65537);assert(input);memset(input,'a',65536);input[65536]=0;
    for(int n=0;n<=65536;++n) {
        char old=input[n];input[n]=0;
        printf("%d %d %d %d\n",n,pw_enc_len(input),pw_enc_blen(input),pw_dec_len(input));
        input[n]=old;
    }
    free(input);
    assert(pw_enc_len(NULL)==-1&&errno==EINVAL);
    assert(pw_enc_blen(NULL)==-1&&errno==EINVAL);
    assert(pw_dec_len(NULL)==-1&&errno==EINVAL);
    assert(nv_pw_enc_size((size_t)INT_MAX-23)==INT_MAX-15);
    assert(nv_pw_enc_size((size_t)INT_MAX-22)==-1&&errno==EOVERFLOW);
    assert(nv_pw_enc_size(SIZE_MAX)==-1&&errno==EOVERFLOW);
    assert(nv_pw_enc_bsize(1610612712)==2147483628);
    assert(nv_pw_enc_bsize(1610612713)==-1&&errno==EOVERFLOW);
    assert(nv_pw_dec_size(2863311528U)==2147483646);
    assert(nv_pw_dec_size(2863311529U)==-1&&errno==EOVERFLOW);
    assert(nv_pw_dec_size(SIZE_MAX)==-1&&errno==EOVERFLOW);
    return 0;
}
