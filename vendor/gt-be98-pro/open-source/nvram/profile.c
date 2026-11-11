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

int nv_profile_write(FILE *output,const char *settings,size_t capacity,unsigned salt) {
    size_t count=0,length,i;unsigned char *encoded,header[8]={'H','D','R','2',0,0,0,0};
    int result=0,saved;
    if(!output || !settings || !capacity || salt>29) {errno=EINVAL;return -1;}
    while(count<capacity && settings[count]) {
        const char *end=memchr(settings+count,0,capacity-count);
        if(!end) {errno=EPROTO;return -1;}
        count=(size_t)(end-settings)+1;
    }
    if(count>=capacity) {errno=EPROTO;return -1;}
    /* Always append padding, including when records end on a block boundary. */
    if(count>0xffffff-1024) {errno=EOVERFLOW;return -1;}
    length=count+(1024-count%1024);
    if(length>capacity) {errno=EOVERFLOW;return -1;}
    encoded=malloc(length);if(!encoded) return -1;
    memset(encoded,0xfe,length);
    for(i=0;i<count;++i) {
        unsigned char c=(unsigned char)settings[i];
        if(c) {
            encoded[i]=(unsigned char)(255-c+salt);
            /* These values decode to NUL; refuse a lossy representation. */
            if(encoded[i]>=0xfd) {free(encoded);errno=EILSEQ;return -1;}
        }
    }
    header[4]=(unsigned char)length;header[5]=(unsigned char)(length>>8);
    header[6]=(unsigned char)(length>>16);header[7]=(unsigned char)salt;
    if(fwrite(header,1,sizeof header,output)!=sizeof header ||
       fwrite(encoded,1,length,output)!=length || fflush(output)) result=-1;
    saved=errno;free(encoded);errno=saved;return result;
}
