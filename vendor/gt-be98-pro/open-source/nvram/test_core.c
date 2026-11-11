/* SPDX-License-Identifier: GPL-2.0-only */
#include "core.h"
#include "frame.h"
#include "pair.h"
#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
struct peer {char value[1024];int present,fail,ignore,gets;};
static int exchange(void *context,const void *request,size_t size,void *reply,size_t capacity,size_t *returned) {
    struct peer *peer=context;struct nv_frame_view frame;struct nv_pair_view pair;
    unsigned char payload[1024];size_t length=0;unsigned type;
    assert(!nv_frame_decode(request,size,&frame));type=frame.type;
    if(peer->fail) {errno=ETIMEDOUT;return -1;}
    if(type==3) {
        assert(frame.length==4 && !memcmp(frame.payload,"key",4));++peer->gets;
        if(peer->present) assert(!nv_pair_encode(payload,sizeof payload,"key",peer->value,strlen(peer->value)+1,&length));
    } else if(type==2 || type==4) {
        assert(!nv_pair_decode(frame.payload,frame.length,&pair));
        assert(!strcmp(pair.name,"key") && pair.value_len<=sizeof peer->value);
        if(!peer->ignore) {
            peer->present=type==2 && pair.value_len;
            if(peer->present) memcpy(peer->value,pair.value,pair.value_len);
        }
        memcpy(payload,frame.payload,frame.length);length=frame.length;
    } else {
        assert(type==5);type=6;
        memcpy(payload,"key=value",10);length=10;
    }
    return nv_frame_encode(reply,capacity,type,0,0,payload,length,returned);
}
static void *stress(void *opaque) {
    struct nv_core *core=opaque;
    for(int i=0;i<100;++i) {
        char *copy=NULL;
        assert(!nv_core_change_ack(core,"key","concurrent",0));
        assert(nv_core_get_copy(core,"key",&copy)==1 && !strcmp(copy,"concurrent"));
        assert(!nv_core_notification(core,NULL,0));
        assert(!strcmp(copy,"concurrent"));free(copy);
    }
    return NULL;
}
int main(void) {
    struct peer peer={0};struct nv_core *core=nv_core_create(exchange,&peer,256*1024);
    char *value=NULL,*retained;unsigned char output[64],message[1280],payload[1024];
    size_t used=0,length,size;pthread_t threads[4];assert(core);
    assert(nv_core_get_copy(core,"key",&value)==0 && !value);
    assert(!nv_core_change_ack(core,"key","",0));
    assert(nv_core_get_copy(core,"key",&value)==1 && !*value);free(value);
    assert(!nv_core_change_ack(core,"key","value",0));
    assert(nv_core_get_copy(core,"key",&retained)==1 && !strcmp(retained,"value"));
    peer.ignore=1;assert(!nv_core_change_ack(core,"key","unapplied",0));
    assert(nv_core_get_copy(core,"key",&value)==1 && !strcmp(value,"value"));free(value);peer.ignore=0;
    peer.fail=1;value=(char *)"unchanged";
    assert(nv_core_get_copy(core,"key",&value)==-1 && errno==ETIMEDOUT && !strcmp(value,"unchanged"));
    assert(nv_core_change_ack(core,"key","unknown",0)==-1 && errno==ETIMEDOUT);peer.fail=0;
    assert(!nv_pair_encode(payload,sizeof payload,"key","unproven",9,&length));
    assert(!nv_frame_encode(message,sizeof message,2,0,0,payload,length,&size));
    assert(nv_core_notification(core,message,size)==1);
    assert(nv_core_get_copy(core,"key",&value)==1 && !strcmp(value,"value"));free(value);
    assert(!nv_core_getall(core,output,sizeof output,&used) && used==10 && !strcmp((char *)output,"key=value"));
    assert(!nv_core_change_ack(core,"key",NULL,1));
    assert(nv_core_get_copy(core,"key",&value)==0 && !value);
    assert(!strcmp(retained,"value"));free(retained);
    for(int i=0;i<4;++i) assert(!pthread_create(&threads[i],NULL,stress,core));
    for(int i=0;i<4;++i) assert(!pthread_join(threads[i],NULL));
    assert(peer.gets>=400);nv_core_destroy(core);
    core=nv_core_create(exchange,&peer,1);assert(core);value=(char *)"unchanged";
    assert(nv_core_get_copy(core,"key",&value)==-1 && errno==ENOSPC && !strcmp(value,"unchanged"));
    nv_core_destroy(core);
    puts("NVRAM core: shared-library GET/SET/UNSET/GETALL, copy ownership, notifications, failures and concurrent calls passed");
    return 0;
}
