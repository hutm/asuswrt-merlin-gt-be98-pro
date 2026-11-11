/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef OPEN_NVRAM_CLIENT_H
#define OPEN_NVRAM_CLIENT_H
#include "frame.h"
#include "pair.h"
#include <errno.h>
#include <stdlib.h>
typedef int (*nv_client_exchange)(void *,const void *,size_t,void *,size_t,size_t *);
static int nv_key_length(const char *key,size_t *length) {
    size_t n=0;
    if(!key || !length) {errno=EINVAL;return -1;}
    while(n<1024 && key[n]) ++n;
    if(n==1024) {errno=EOVERFLOW;return -1;}
    *length=n+1;return 0;
}
/* Return 1 with malloc-owned value, 0 with NULL for missing, -1 for error.
 * This internal API does not yet implement nvram_get's retained-pointer ABI. */
static int nv_client_get(nv_client_exchange exchange,void *context,uint32_t pid,
                         const char *key,char **value) {
    unsigned char request[1280],reply[1280];
    size_t key_length,request_size,reply_size=0;
    struct nv_frame_view frame;struct nv_pair_view pair;char *copy;
    if(!exchange || !value) {errno=EINVAL;return -1;}
    if(nv_key_length(key,&key_length)) return -1;
    if(nv_frame_encode(request,sizeof request,3,pid,0,key,key_length,&request_size)) {errno=EOVERFLOW;return -1;}
    if(exchange(context,request,request_size,reply,sizeof reply,&reply_size)) return -1;
    if(reply_size>sizeof reply || nv_frame_decode(reply,reply_size,&frame) ||
       frame.consumed!=reply_size || frame.type!=3) {errno=EPROTO;return -1;}
    if(!frame.length) {*value=NULL;return 0;}
    if(nv_pair_decode(frame.payload,frame.length,&pair) || pair.consumed!=frame.length ||
       strcmp(pair.name,key)) {errno=EPROTO;return -1;}
    if(!pair.value_len) {*value=NULL;return 0;}
    if(pair.value[pair.value_len-1] || memchr(pair.value,0,pair.value_len-1)) {errno=EPROTO;return -1;}
    copy=malloc(pair.value_len);if(!copy) return -1;
    memcpy(copy,pair.value,pair.value_len);*value=copy;return 1;
}
/* Acknowledgement only: the kernel echoes SET/UNSET without propagating its
 * allocation/set error status. A successful ack does not prove applied state
 * or persistence. Notifications and readback policy require separate handling. */
static int nv_client_change_ack(nv_client_exchange exchange,void *context,uint32_t pid,
                                const char *key,const char *value,int unset) {
    unsigned char pair[1024],request[1280],reply[1280];
    size_t key_length,value_length=0,pair_length,request_size,reply_size=0;
    uint16_t type=unset?4:2;struct nv_frame_view frame;
    if(!exchange || (unset && value)) {errno=EINVAL;return -1;}
    if(nv_key_length(key,&key_length)) return -1;
    (void)key_length;
    if(value) {
        while(value_length<sizeof pair && value[value_length]) ++value_length;
        if(value_length==sizeof pair) {errno=EOVERFLOW;return -1;}
        ++value_length;
    }
    if(nv_pair_encode(pair,sizeof pair,key,value,value_length,&pair_length) || pair_length>=1024 ||
       nv_frame_encode(request,sizeof request,type,pid,0,pair,pair_length,&request_size)) {errno=EOVERFLOW;return -1;}
    if(exchange(context,request,request_size,reply,sizeof reply,&reply_size)) return -1;
    if(reply_size>sizeof reply || nv_frame_decode(reply,reply_size,&frame) || frame.consumed!=reply_size ||
       frame.type!=type || frame.length!=pair_length || memcmp(frame.payload,pair,pair_length)) {errno=EPROTO;return -1;}
    return 0;
}
#endif
