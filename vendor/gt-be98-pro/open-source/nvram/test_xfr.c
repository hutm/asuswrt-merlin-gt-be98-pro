/* SPDX-License-Identifier: GPL-2.0-only */
#include "xfr.h"
#include <assert.h>
#include <stdio.h>
static int mode,calls;static const char *expected;
static int exchange(void *context,const void *request,size_t size,void *reply,size_t capacity,size_t *returned) {
    struct nv_frame_view f;unsigned char payload[1025];size_t n=3;unsigned type=14;(void)context;
    assert(!nv_frame_decode(request,size,&f));assert(f.type==14&&f.length==strlen(expected)+1&&!strcmp((const char *)f.payload,expected));++calls;
    memset(payload,'x',sizeof payload);memcpy(payload,"abc",3);
    if(mode==1) n=0;
    if(mode==2) {payload[1]=0;n=3;}
    if(mode==3) n=1024;
    if(mode==4) n=1025;
    if(mode==5) type=3;
    if(mode==6) {errno=ETIMEDOUT;return -1;}
    if(mode==7) {*returned=capacity+1;return 0;}
    return nv_frame_encode(reply,capacity,type,0,0,payload,n,returned);
}
int main(void) {
    char out[1025],input[513];expected="request";
    for(mode=0;mode<8;++mode) {
        strcpy(out,"unchanged");int r=nv_xfr(exchange,NULL,expected,out);
        if(mode==1) assert(!r&&!strcmp(out,"unchanged"));
        else if(mode>=4) assert(r==-1&&!strcmp(out,"unchanged")&&errno==(mode==6?ETIMEDOUT:EPROTO));
        else if(mode==2) assert(r==1&&!strcmp(out,"a"));
        else if(mode==3) assert(r==1&&strlen(out)==1024);
        else assert(r==1&&!strcmp(out,"abc"));
    }
    mode=0;expected="";assert(nv_xfr(exchange,NULL,expected,out)==1);
    memset(input,'a',sizeof input);input[511]=0;expected=input;assert(nv_xfr(exchange,NULL,input,out)==1);
    int before=calls;input[511]='a';input[512]=0;assert(nv_xfr(exchange,NULL,input,out)==-1&&errno==EOVERFLOW&&calls==before);
    assert(nv_xfr(exchange,NULL,NULL,out)==-1&&errno==EINVAL);
    puts("NVRAM conversion: request limits, bounded replies, malformed frames and failure preservation passed");return 0;
}
