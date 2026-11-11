/* SPDX-License-Identifier: GPL-2.0-only */
#include "lengths.h"
#include <errno.h>
#include <limits.h>
#include <string.h>
int nv_pw_enc_size(size_t length) {
    /* Eight bytes of wrapper overhead, rounded up to a complete AES block. */
    if(length>(size_t)INT_MAX-23) {errno=EOVERFLOW;return -1;}
    return (int)((length+23)&~(size_t)15);
}
int nv_pw_enc_bsize(size_t length) {
    int bytes=nv_pw_enc_size(length);size_t groups;
    if(bytes<0) return -1;
    groups=((size_t)bytes+2)/3;
    if(groups>(size_t)INT_MAX/4) {errno=EOVERFLOW;return -1;}
    return (int)(groups*4);
}
int nv_pw_dec_size(size_t length) {
    size_t groups=length/4+(length%4!=0);
    if(groups>(size_t)INT_MAX/3) {errno=EOVERFLOW;return -1;}
    return (int)(groups*3);
}
int pw_enc_len(const char *input) {
    if(!input) {errno=EINVAL;return -1;}
    return nv_pw_enc_size(strlen(input));
}
int pw_enc_blen(const char *input) {
    if(!input) {errno=EINVAL;return -1;}
    return nv_pw_enc_bsize(strlen(input));
}
int pw_dec_len(const char *input) {
    if(!input) {errno=EINVAL;return -1;}
    return nv_pw_dec_size(strlen(input));
}
