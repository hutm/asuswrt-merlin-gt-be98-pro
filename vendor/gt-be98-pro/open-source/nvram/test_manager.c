/* SPDX-License-Identifier: GPL-2.0-only */
#include "manager.h"
#include <assert.h>
#include <stdio.h>
int main(void) {
    unsigned char request[128],reply[128];size_t size;struct nv_frame_view f;uint32_t flags,inner;
    assert(!nv_restart_request(request,sizeof request,3,2,1,1,&size));
    assert(!nv_frame_decode(request,size,&f)&&f.type==9&&f.length==36);
    memcpy(&flags,f.payload,4);memcpy(&inner,request+20,4);
    assert(flags==0x03020603&&inner==1&&!memcmp(f.payload+24,"restart",8));
    for(unsigned i=4;i<20;++i) assert(!f.payload[i]);
    for(unsigned i=32;i<36;++i) assert(!f.payload[i]);
    assert(nv_restart_request(request,1,0,0,0,0,&size)==-1);
    struct nv_restart_reply state={0},before;
    assert(!nv_frame_encode(reply,sizeof reply,10,0,42,"part",4,&size));
    assert(!nv_restart_accept(&state,reply,size)&&state.used==4);
    before=state;
    assert(!nv_frame_encode(reply,sizeof reply,11,0,43,"done",4,&size));
    assert(nv_restart_accept(&state,reply,size)==-1&&errno==EPROTO&&!memcmp(&state,&before,sizeof state));
    assert(!nv_frame_encode(reply,sizeof reply,11,0,42,"done",4,&size));
    assert(nv_restart_accept(&state,reply,size)==1&&state.used==8&&!memcmp(state.data,"partdone",8));
    assert(nv_restart_accept(&state,reply,size)==-1&&errno==EINVAL);
    for(int mode=0;mode<4;++mode){
        memset(&state,0,sizeof state);before=state;
        assert(!nv_frame_encode(reply,sizeof reply,mode==0?9:11,0,mode==1?0:42,"01234567890123456",mode==2?0:mode==3?17:1,&size));
        assert(nv_restart_accept(&state,reply,size)==-1&&!memcmp(&state,&before,sizeof state));
    }
    puts("Manager restart: request fields, multipart replies, sender consistency and bounded accumulation passed");return 0;
}
