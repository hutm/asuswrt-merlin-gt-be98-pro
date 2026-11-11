/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef OPEN_NVRAM_GETALL_H
#define OPEN_NVRAM_GETALL_H
#include "frame.h"
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
typedef int (*nv_exchange_fn)(void *, const void *, size_t, void *, size_t, size_t *);
/* Transport callback must authenticate origin, enforce a timeout, and serialize
 * this request/reply exchange. Kernel state may change between pages: this API
 * cannot provide snapshot consistency. Output remains unchanged on failure. */
static int nv_getall_pages(nv_exchange_fn exchange, void *context, uint32_t pid,
                           void *output, size_t capacity, size_t *used)
{
    unsigned char request[32],reply[1024],*scratch;
    size_t total=0,request_size,reply_size;
    int32_t args[2];
    struct nv_frame_view view;
    int result=-1;
    if(!exchange || !output || !used || !capacity || capacity>INT_MAX) {
        errno=EINVAL;return -1;
    }
    scratch=malloc(capacity);if(!scratch) return -1;
    args[0]=(int32_t)capacity;
    for(args[1]=0;;++args[1]) {
        if(nv_frame_encode(request,sizeof request,5,pid,0,args,sizeof args,&request_size)) {
            errno=EINVAL;break;
        }
        reply_size=0;
        if(exchange(context,request,request_size,reply,sizeof reply,&reply_size)) break;
        if(reply_size>sizeof reply || nv_frame_decode(reply,reply_size,&view) ||
           view.consumed!=reply_size || (view.type!=5 && view.type!=6) ||
           (view.type==5 && view.length!=1000) ||
           (view.type==6 && view.length>=1000)) { errno=EPROTO;break; }
        if(view.length>capacity-total) { errno=ENOSPC;break; }
        memcpy(scratch+total,view.payload,view.length);total+=view.length;
        if(view.type==6) {
            if(total && scratch[total-1]!=0) { errno=EPROTO;break; }
            memcpy(output,scratch,total);*used=total;result=0;break;
        }
        /* The kernel multiplies the page index by 1000 in a signed int. */
        if(args[1]>=INT_MAX/1000) { errno=EOVERFLOW;break; }
    }
    free(scratch);return result;
}
#endif
