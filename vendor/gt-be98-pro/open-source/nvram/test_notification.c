/* SPDX-License-Identifier: GPL-2.0-only */
#include "notification.h"
#include <assert.h>
#include <stdio.h>
struct events {int keys,all;};
static void invalidate(void *context,const char *key) {
    struct events *events=context;
    if(key) {assert(!strcmp(key,"key"));++events->keys;}
    else ++events->all;
}
int main(void) {
    unsigned char payload[1024],message[1280];size_t length,size;
    struct events events={0};
    assert(!nv_pair_encode(payload,sizeof payload,"key","unproven",9,&length));
    for(unsigned type=2;type<=4;type+=2) {
        assert(!nv_frame_encode(message,sizeof message,type,12345,6789,payload,length,&size));
        assert(nv_notification(message,size,invalidate,&events)==1);
        for(size_t cut=0;cut<size;++cut)
            assert(nv_notification(message,cut,invalidate,&events)==-1 && errno==EPROTO);
    }
    assert(events.keys==2 && events.all>0);
    int count=events.all;
    assert(!nv_frame_encode(message,sizeof message,7,12345,0,NULL,0,&size));
    assert(nv_notification(message,size,invalidate,&events)==2 && events.all==count);
    assert(!nv_frame_encode(message,sizeof message,3,12345,0,NULL,0,&size));
    assert(nv_notification(message,size,invalidate,&events)==0 && events.all==count);
    assert(!nv_frame_encode(message,sizeof message,7,12345,0,payload,length,&size));
    assert(nv_notification(message,size,invalidate,&events)==-1 && events.all==count+1);
    payload[0]=0;payload[1]=payload[2]=payload[3]=0;
    assert(!nv_frame_encode(message,sizeof message,2,12345,0,payload,length,&size));
    assert(nv_notification(message,size,invalidate,&events)==-1 && events.all==count+2);
    puts("NVRAM notifications: key invalidation, malformed/truncated frames, commit and unrelated events passed");
    return 0;
}
