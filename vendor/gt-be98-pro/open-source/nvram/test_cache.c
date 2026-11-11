/* SPDX-License-Identifier: GPL-2.0-only */
#include "cache.h"
#include <assert.h>
#include <stdio.h>
struct reply {int state;const char *value;};
static int fetch(void *context,const char *key,char **value) {
    struct reply *reply=context;assert(!strcmp(key,"key"));
    if(reply->state<0) {errno=ETIMEDOUT;return -1;}
    if(reply->value) {
        *value=malloc(strlen(reply->value)+1);assert(*value);
        strcpy(*value,reply->value);
    } else *value=NULL;
    return reply->state;
}
int main(void) {
    struct nv_cache cache={.limit=256*1024};
    char *first,*value,*other;char large[1025],key[40];size_t before;
    assert(nv_cache_lookup(&cache,"key",&value)==0 && !value);
    assert(nv_cache_store(&cache,"key","first",&first)==1);
    assert(nv_cache_store(&cache,"key","two",&value)==1 && value==first);
    assert(!strcmp(first,"two"));
    memset(large,'x',1023);large[1023]=0;
    assert(nv_cache_store(&cache,"key",large,&value)==1 && value==first);
    assert(!strcmp(first,large));
    assert(nv_cache_store(&cache,"key",first+1,&value)==1 && strlen(first)==1022);
    before=cache.used;value=(char *)"unchanged";
    large[1023]='x';large[1024]=0;
    assert(nv_cache_store(&cache,"key",large,&value)==-1 && errno==EOVERFLOW);
    assert(cache.used==before && !strcmp(value,"unchanged") && strlen(first)==1022);
    assert(nv_cache_store(&cache,"key",NULL,&value)==0 && !value && !*first);
    assert(nv_cache_lookup(&cache,"key",&value)==0 && !value);
    assert(nv_cache_store(&cache,"key","",&value)==1 && value==first && !*value);
    assert(nv_cache_lookup(&cache,"key",&value)==1 && value==first);
    /* Colliding buckets and many unrelated updates must preserve the address. */
    for(int i=0;i<100;++i) {
        snprintf(key,sizeof key,"key-%d",i);
        assert(nv_cache_store(&cache,key,key,&other)==1);
        assert(!strcmp(other,key));
    }
    assert(nv_cache_store(&cache,"key","restored",&value)==1 && value==first);
    cache.limit=cache.used;
    value=(char *)"unchanged";
    assert(nv_cache_store(&cache,"new","value",&value)==-1 && errno==ENOSPC);
    assert(!strcmp(value,"unchanged") && !strcmp(first,"restored"));
    assert(nv_cache_store(&cache,"unknown",NULL,&value)==0 && !value);
    assert(nv_cache_store(&cache,"key","existing",&value)==1 && value==first);
    {
        struct reply reply={-1,NULL};value=(char *)"unchanged";
        assert(nv_cache_refresh(&cache,fetch,&reply,"key",&value)==-1 && errno==ETIMEDOUT);
        assert(!strcmp(value,"unchanged") && !strcmp(first,"existing"));
        reply.state=1;reply.value="fresh";
        assert(nv_cache_refresh(&cache,fetch,&reply,"key",&value)==1 && value==first);
        assert(!strcmp(first,"fresh"));
        reply.state=0;reply.value=NULL;
        assert(nv_cache_refresh(&cache,fetch,&reply,"key",&value)==0 && !value && !*first);
        reply.state=1;value=(char *)"unchanged";
        assert(nv_cache_refresh(&cache,fetch,&reply,"key",&value)==-1 && errno==EPROTO);
        assert(!strcmp(value,"unchanged"));
    }
    nv_cache_destroy(&cache);assert(!cache.used);
    assert(nv_cache_lookup(&cache,"key",&value)==0 && !value);
    nv_cache_destroy(&cache);
    puts("NVRAM cache: retained addresses, growth, absence/empty, aliasing, collisions and memory bounds passed");
    return 0;
}
