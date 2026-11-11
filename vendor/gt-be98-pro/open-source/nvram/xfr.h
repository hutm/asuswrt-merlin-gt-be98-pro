/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef OPEN_NVRAM_XFR_H
#define OPEN_NVRAM_XFR_H
#include "frame.h"
#include <errno.h>
/* Conversion is provided by the source kernel NLS handler. The caller owns a
 * 1025-byte buffer. Return 1 for a value, 0 for empty reply, -1 for error.
 * Missing/error replies preserve the caller buffer. */
static int nv_xfr(int (*exchange)(void *,const void *,size_t,void *,size_t,size_t *),
                  void *context,const char *input,char output[1025]) {
    unsigned char request[1280],reply[1280];size_t n=0,size,received=0;
    struct nv_frame_view frame;const unsigned char *end;
    if(!exchange || !input || !output) {errno=EINVAL;return -1;}
    while(n<512 && input[n]) ++n;
    if(n==512) {errno=EOVERFLOW;return -1;}
    if(nv_frame_encode(request,sizeof request,14,0,0,input,n+1,&size)) {errno=EOVERFLOW;return -1;}
    if(exchange(context,request,size,reply,sizeof reply,&received)) return -1;
    if(received>sizeof reply || nv_frame_decode(reply,received,&frame) ||
       frame.consumed!=received || frame.type!=14 || frame.length>1024) {errno=EPROTO;return -1;}
    if(!frame.length) return 0;
    end=memchr(frame.payload,0,frame.length);
    n=end?(size_t)(end-frame.payload):frame.length;
    memcpy(output,frame.payload,n);output[n]=0;return 1;
}
#endif
