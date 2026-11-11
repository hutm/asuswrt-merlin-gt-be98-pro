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

int nv_pw_unframe(const unsigned char *input,size_t length,char *output,size_t capacity) {
    static const char hex[]="0123456789abcdef";
    size_t text_length;const unsigned char *end;uint32_t crc;int padded;
    if(!output || !capacity || (length && !input)) {errno=EINVAL;return -1;}
    if(!length) {output[0]=0;return 0;}
    if(length<16 || length%16) {errno=EPROTO;return -1;}
    end=memchr(input+8,0,length-8);
    text_length=end?(size_t)(end-input)-8:length-8;
    if(!text_length) {errno=EPROTO;return -1;}
    padded=nv_pw_enc_size(text_length);if(padded<0) return -1;
    if((size_t)padded!=length) {errno=EPROTO;return -1;}
    for(size_t i=8+text_length;i<length;++i)
        if(input[i]) {errno=EPROTO;return -1;}
    crc=(uint32_t)crc32(0,input+8,(uInt)text_length);
    for(unsigned i=0;i<8;++i)
        if(input[i]!=(unsigned char)hex[(crc>>(28-i*4))&15]) {errno=EBADMSG;return -1;}
    if(text_length>=capacity) {errno=ENOSPC;return -1;}
    memcpy(output,input+8,text_length);output[text_length]=0;return 0;
}
