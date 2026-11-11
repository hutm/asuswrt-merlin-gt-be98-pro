/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef OPEN_NVRAM_MANAGER_H
#define OPEN_NVRAM_MANAGER_H
#include "frame.h"
#include <errno.h>
/* Restart uses manager GET (9), not the ordinary NVRAM request protocol.
 * The kernel forwards responses 10/11 and supplies the manager's port ID in
 * the inner header. Transport-origin validation must precede this decoder. */
static int nv_restart_request(void *output,size_t capacity,uint32_t radio,
                              uint32_t source,int persist,int wait,size_t *size) {
    unsigned char payload[36]={0};uint32_t flags,n=8;
    flags=(radio&~UINT32_C(0x00ff0000))|(source<<16);
    if(persist) flags|=UINT32_C(0x02000000);
    if(wait) flags|=UINT32_C(0x01000000);
    flags=(flags&~UINT32_C(0x0000ff00))|UINT32_C(0x600);
    memcpy(payload,&flags,4);
    memcpy(payload+20,&n,4);memcpy(payload+24,"restart",8);
    /* Zero reserved header bytes; the reference leaves these uninitialized. */
    if(nv_frame_encode(output,capacity,9,0,1,payload,sizeof payload,size)) {
        errno=EINVAL;return -1;
    }
    return 0;
}
struct nv_restart_reply {uint32_t manager;size_t used;int done;unsigned char data[16];};
/* 0: accepted continuation, 1: complete, -1: malformed/overflow.
 * On error the state is unchanged. Completion is an acknowledgement only. */
static int nv_restart_accept(struct nv_restart_reply *state,const void *data,size_t size) {
    struct nv_frame_view frame;uint32_t manager;
    if(!state || !data || state->done || state->used>sizeof state->data) {errno=EINVAL;return -1;}
    if(nv_frame_decode(data,size,&frame) || frame.consumed!=size ||
       (frame.type!=10 && frame.type!=11) || !frame.length) {errno=EPROTO;return -1;}
    memcpy(&manager,(const unsigned char *)data+20,4);
    if(!manager || (state->manager && state->manager!=manager)) {errno=EPROTO;return -1;}
    if(frame.length>sizeof state->data-state->used) {errno=EOVERFLOW;return -1;}
    memcpy(state->data+state->used,frame.payload,frame.length);
    state->used+=frame.length;state->manager=manager;state->done=frame.type==11;
    return state->done;
}
#endif
