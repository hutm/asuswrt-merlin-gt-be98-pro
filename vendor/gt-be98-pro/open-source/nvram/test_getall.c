#include "getall.h"
#include <assert.h>
#include <stdio.h>
struct mock { unsigned char data[3000]; size_t length,calls; int mode; };
static int exchange(void *context,const void *request,size_t size,void *reply,size_t capacity,size_t *returned) {
    struct mock *m=context;
    struct nv_frame_view view;
    int32_t args[2];
    size_t offset,length;
    uint16_t type;
    assert(!nv_frame_decode(request,size,&view) && view.type==5 && view.length==8);
    memcpy(args,view.payload,8);
    assert(args[1]==(int32_t)m->calls && args[0]>0);
    ++m->calls;
    if(m->mode==1 && m->calls==2) { errno=ETIMEDOUT;return -1; }
    offset=(size_t)args[1]*1000;
    length=offset<m->length?m->length-offset:0;
    type=length>=1000?5:6;
    if(length>1000) length=1000;
    if(m->mode==2) type=3;
    if(m->mode==3) { type=5;length=0; }
    assert(!nv_frame_encode(reply,capacity,type,0,0,m->data+offset,length,returned));
    if(m->mode==4) --*returned;
    return 0;
}
int main(void) {
    struct mock m;
    unsigned char output[3000];
    size_t sizes[]={0,1,999,1000,1001,1999,2000,2999},used;
    for(size_t i=0;i<sizeof sizes/sizeof sizes[0];++i) {
        memset(&m,0,sizeof m);m.length=sizes[i];
        memset(m.data,'a',sizeof m.data);
        if(m.length) m.data[m.length-1]=0;
        used=9999;
        assert(!nv_getall_pages(exchange,&m,42,output,sizeof output,&used));
        assert(used==m.length && !memcmp(output,m.data,used));
        assert(m.calls==m.length/1000+1);
    }
    for(int mode=1;mode<=4;++mode) {
        memset(&m,0,sizeof m);m.mode=mode;m.length=2000;
        memset(output,0x55,sizeof output);used=8888;
        assert(nv_getall_pages(exchange,&m,42,output,sizeof output,&used)==-1);
        assert(used==8888);
        for(size_t i=0;i<sizeof output;++i) assert(output[i]==0x55);
    }
    memset(&m,0,sizeof m);m.length=2000;used=8888;
    assert(nv_getall_pages(exchange,&m,42,output,1500,&used)==-1 && errno==ENOSPC && used==8888);
    memset(&m,0,sizeof m);m.length=1;m.data[0]='x';
    assert(nv_getall_pages(exchange,&m,42,output,sizeof output,&used)==-1 && errno==EPROTO);
    puts("NVRAM paging: empty/exact/multiple pages, malformed replies, timeout and unchanged output on failure passed");
    return 0;
}
