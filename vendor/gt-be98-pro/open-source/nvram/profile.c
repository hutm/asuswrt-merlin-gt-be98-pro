/* SPDX-License-Identifier: GPL-2.0-only */
#include "profile.h"
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
int nv_profile_read(FILE *input,size_t capacity,int (*apply)(void *,const char *,const char *),void *context) {
    unsigned char header[8],*data;size_t length,offset;int encoded,result=0,saved;
    if(!input || !apply || !capacity) {errno=EINVAL;return -1;}
    if(fread(header,1,sizeof header,input)!=sizeof header) {if(!ferror(input)) errno=EPROTO;return -1;}
    encoded=!memcmp(header,"HDR2",4);
    if(!encoded && memcmp(header,"HDR1",4)) {errno=EPROTO;return -1;}
    length=(uint32_t)header[4]|((uint32_t)header[5]<<8)|((uint32_t)header[6]<<16);
    if(!encoded) length|=(uint32_t)header[7]<<24;
    if(length>capacity) {errno=EOVERFLOW;return -1;}
    data=malloc(length?length:1);if(!data) return -1;
    if(fread(data,1,length,input)!=length) {if(!ferror(input)) errno=EPROTO;result=-1;goto done;}
    if(encoded) for(offset=0;offset<length;++offset)
        data[offset]=data[offset]>=0xfd?0:(unsigned char)(0xff+header[7]-data[offset]);
    /* Validate string boundaries before the first callback. */
    offset=0;
    while(offset<length) {
        if(!isprint(data[offset])) {++offset;continue;}
        unsigned char *end=memchr(data+offset,0,length-offset);
        if(!end) {errno=EPROTO;result=-1;goto done;}
        offset=(size_t)(end-data)+1;
    }
    offset=0;
    while(offset<length) {
        if(!isprint(data[offset])) {++offset;continue;}
        char *key=(char *)data+offset,*value=strchr(key,'=');
        offset+=strlen(key)+1;
        if(value) *value++=0;
        if(apply(context,key,value)) {result=-1;break;}
    }
done:
    saved=errno;free(data);errno=saved;return result;
}
