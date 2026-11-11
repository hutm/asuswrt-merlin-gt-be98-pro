/* SPDX-License-Identifier: GPL-2.0-only */
#include "frame.h"
#include "lengths.h"
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <zlib.h>
int nv_pw_frame(const char *input,unsigned char *output,size_t capacity,size_t *written) {
    static const char hex[]="0123456789abcdef";
    size_t length;int padded;uint32_t crc;
    if(written) *written=0;
    if(!input || !written) {errno=EINVAL;return -1;}
    length=strlen(input);if(!length) return 0;
    padded=nv_pw_enc_size(length);if(padded<0) return -1;
    if(!output) {errno=EINVAL;return -1;}
    if(capacity<(size_t)padded) {errno=ENOSPC;return -1;}
    crc=(uint32_t)crc32(0,(const unsigned char *)input,(uInt)length);
    memset(output,0,(size_t)padded);
    for(unsigned i=0;i<8;++i)output[i]=(unsigned char)hex[(crc>>(28-i*4))&15];
    memcpy(output+8,input,length);*written=(size_t)padded;return 0;
}
