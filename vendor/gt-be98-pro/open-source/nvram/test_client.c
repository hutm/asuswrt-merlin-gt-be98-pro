#include "client.h"
#include <assert.h>
#include <stdio.h>
struct mock {int mode;const char *value;int calls;};
static int exchange(void *context,const void *request,size_t size,void *reply,size_t capacity,size_t *returned) {
    struct mock *m=context;struct nv_frame_view frame;struct nv_pair_view pair;
    unsigned char encoded[1024];size_t length=0;
    ++m->calls;assert(!nv_frame_decode(request,size,&frame));
    if(m->mode==1) {errno=ETIMEDOUT;return -1;}
    if(frame.type==3) {
        assert(!strcmp((const char *)frame.payload,"key"));
        if(m->value) assert(!nv_pair_encode(encoded,sizeof encoded,m->mode==2?"wrong":"key",m->value,strlen(m->value)+1,&length));
        assert(!nv_frame_encode(reply,capacity,3,0,0,encoded,length,returned));
    } else {
        assert(frame.type==2 || frame.type==4);
        assert(!nv_pair_decode(frame.payload,frame.length,&pair));assert(!strcmp(pair.name,"key"));
        if(frame.type==4) assert(!pair.value);
        else if(m->value) assert(pair.value && !strcmp((const char *)pair.value,m->value));
        assert(!nv_frame_encode(reply,capacity,frame.type,0,0,frame.payload,frame.length,returned));
    }
    if(m->mode==3) --*returned;
    return 0;
}
int main(void) {
    struct mock m={0,"value",0};char *value=NULL;char large[1012];
    assert(nv_client_get(exchange,&m,5,"key",&value)==1 && !strcmp(value,"value"));free(value);
    m.value="";assert(nv_client_get(exchange,&m,5,"key",&value)==1 && !*value);free(value);
    m.value=NULL;assert(nv_client_get(exchange,&m,5,"key",&value)==0 && !value);
    m.value="value";
    for(int mode=1;mode<=3;++mode) {
        m.mode=mode;value=(char *)"unchanged";
        assert(nv_client_get(exchange,&m,5,"key",&value)==-1 && !strcmp(value,"unchanged"));
    }
    m.mode=0;assert(!nv_client_change_ack(exchange,&m,5,"key","value",0));
    assert(!nv_client_change_ack(exchange,&m,5,"key",NULL,1));
    memset(large,'x',sizeof large-1);large[sizeof large-1]=0;m.value=large;
    /* 1024-byte pair is rejected by the kernel. 1023 bytes is allowed and
     * requires more than a 1024-byte total Netlink transport buffer. */
    int calls=m.calls;assert(nv_client_change_ack(exchange,&m,5,"key",large,0)==-1 && m.calls==calls);
    large[sizeof large-2]=0;assert(!nv_client_change_ack(exchange,&m,5,"key",large,0));
    m.mode=3;assert(nv_client_change_ack(exchange,&m,5,"key",large,0)==-1);
    puts("NVRAM client: get/missing/empty, SET/UNSET pair encoding, malformed replies and maximum payload passed");return 0;
}
