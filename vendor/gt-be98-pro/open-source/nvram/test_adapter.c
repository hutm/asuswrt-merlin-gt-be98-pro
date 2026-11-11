/* SPDX-License-Identifier: GPL-2.0-only */
#include "adapter.h"
#include "frame.h"
#include "pair.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
struct state {char small[64],large[64];int present,denied,lock_failed,locked,unlocks,exchanges,last_type,bad_all;};
static int exchange(void *opaque,const void *request,size_t size,void *reply,size_t capacity,size_t *returned) {
 struct state *s=opaque;struct nv_frame_view f;struct nv_pair_view p;unsigned char payload[1024];size_t n=0;
 assert(!nv_frame_decode(request,size,&f));++s->exchanges;s->last_type=f.type;
 if(f.type==3){assert(!strcmp((const char *)f.payload,"small"));if(s->present)assert(!nv_pair_encode(payload,sizeof payload,"small",s->small,strlen(s->small)+1,&n));}
 else if(f.type==2||f.type==4){assert(!nv_pair_decode(f.payload,f.length,&p));assert(!strcmp(p.name,"small"));s->present=f.type==2;if(s->present){assert(p.value_len<=sizeof s->small);memcpy(s->small,p.value,p.value_len);}memcpy(payload,f.payload,f.length);n=f.length;}
 else {assert(f.type==5);memcpy(payload,"small=one",10);n=10;}
 return nv_frame_encode(reply,capacity,f.type==5?6:f.type,0,0,payload,n,returned);
}
static int is_large(void *opaque,const char *key){(void)opaque;return !strcmp(key,"large");}
static int lock_storage(void *opaque){struct state *s=opaque;if(s->lock_failed){errno=EBUSY;return -1;}assert(!s->locked);s->locked=1;return 0;}
static void unlock_storage(void *opaque){struct state *s=opaque;assert(s->locked);s->locked=0;++s->unlocks;errno=0;}
static char *get(void *opaque,const char *key){struct state *s=opaque;assert(s->locked&&!strcmp(key,"large"));return s->large;}
static int set(void *opaque,const char *key,const char *value){struct state *s=opaque;assert(s->locked&&!strcmp(key,"large"));if(value){assert(strlen(value)<sizeof s->large);strcpy(s->large,value);return (int)strlen(value);}s->large[0]=0;return 0;}
static int unset(void *opaque,const char *key){return set(opaque,key,NULL);}
static int getall(void *opaque,int used,char *out,int capacity){struct state *s=opaque;assert(s->locked&&used==10);if(s->bad_all){errno=EIO;return s->bad_all==1?-1:capacity;}assert(capacity>used+10);memcpy(out+used,"large=two",10);return used+10;}
static int deny_get(void *opaque,const char *key){(void)key;return ((struct state *)opaque)->denied;}
static int deny_all(void *opaque){return ((struct state *)opaque)->denied;}
int main(void){
 struct state state={0};struct nv_core *core=nv_core_create(exchange,&state,65536);struct nv_adapter a;
 struct nv_storage_hooks hooks={&state,is_large,lock_storage,unlock_storage,get,set,unset,getall,deny_get,deny_all};assert(core);
 assert(!nv_adapter_init(&a,core,&hooks));
 assert(!nv_adapter_get(&a,"small"));assert(!nv_adapter_set(&a,"small","one"));char *borrowed=nv_adapter_get(&a,"small");assert(borrowed&&!strcmp(borrowed,"one"));
 assert(!nv_adapter_set(&a,"small","two"));assert(nv_adapter_get(&a,"small")==borrowed&&!strcmp(borrowed,"two"));
 assert(!nv_adapter_set(&a,"small",NULL)&&state.last_type==4);assert(!nv_adapter_get(&a,"small")&&!*borrowed);
 assert(!nv_adapter_unset(&a,"small")&&state.last_type==4);
 int requests=state.exchanges;assert(nv_adapter_set(&a,"large","larger")==6);assert(!strcmp(nv_adapter_get(&a,"large"),"larger"));assert(!nv_adapter_unset(&a,"large"));assert(requests==state.exchanges);
 state.lock_failed=1;int unlocks=state.unlocks;assert(nv_adapter_set(&a,"large","x")==-1&&errno==EBUSY);assert(!nv_adapter_get(&a,"large")&&errno==EBUSY);assert(state.unlocks==unlocks);state.lock_failed=0;
 state.denied=1;requests=state.exchanges;assert(!nv_adapter_get(&a,"small")&&errno==EACCES);char out[64];memset(out,0x55,sizeof out);assert(nv_adapter_getall(&a,out,sizeof out)==-1&&errno==EACCES&&out[0]==0x55&&state.exchanges==requests);state.denied=0;
 assert(nv_adapter_getall(&a,out,sizeof out)==20&&!memcmp(out,"small=one\0large=two\0",20));for(int i=20;i<64;++i)assert(!out[i]);
 for(int error=1;error<=2;++error){state.bad_all=error;memset(out,0x55,sizeof out);assert(nv_adapter_getall(&a,out,sizeof out)==-1&&errno==(error==1?EIO:EPROTO));for(int i=0;i<64;++i)assert(out[i]==0x55);}state.bad_all=0;
 assert(nv_adapter_getall(&a,out,10)==-1&&errno==ENOSPC);
 hooks.lock=NULL;assert(nv_adapter_init(&a,core,&hooks)==-1&&errno==EINVAL);
 nv_core_destroy(core);puts("NVRAM adapter: backend routing, borrowed pointers, permissions, locks, byte-count enumeration and failure preservation passed");return 0;
}
