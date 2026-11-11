/* SPDX-License-Identifier: GPL-2.0-only */
#define _POSIX_C_SOURCE 200809L
#include "core.h"
#include "client.h"
#include "cache.h"
#include "getall.h"
#include "notification.h"
#include "persist.h"
#include <pthread.h>
struct nv_core {
    pthread_mutex_t mutex;
    struct nv_cache cache;
    nv_core_exchange exchange;
    void *context;
};
static int lock(struct nv_core *core) {
    int error;
    if(!core) {errno=EINVAL;return -1;}
    error=pthread_mutex_lock(&core->mutex);
    if(error) {errno=error;return -1;}return 0;
}
static void unlock(struct nv_core *core) {
    int saved=errno;pthread_mutex_unlock(&core->mutex);errno=saved;
}
struct nv_core *nv_core_create(nv_core_exchange exchange,void *context,size_t cache_limit) {
    struct nv_core *core;int error;
    if(!exchange || !cache_limit) {errno=EINVAL;return NULL;}
    core=calloc(1,sizeof *core);if(!core) return NULL;
    error=pthread_mutex_init(&core->mutex,NULL);
    if(error) {free(core);errno=error;return NULL;}
    core->exchange=exchange;core->context=context;core->cache.limit=cache_limit;
    return core;
}
void nv_core_destroy(struct nv_core *core) {
    if(!core) return;
    nv_cache_destroy(&core->cache);pthread_mutex_destroy(&core->mutex);free(core);
}
static int fetch(void *opaque,const char *key,char **value) {
    struct nv_core *core=opaque;
    return nv_client_get(core->exchange,core->context,0,key,value);
}
int nv_core_get_copy(struct nv_core *core,const char *key,char **value) {
    char *retained,*copy;int result;
    if(!key || !value) {errno=EINVAL;return -1;}
    if(lock(core)) return -1;
    result=nv_cache_refresh(&core->cache,fetch,core,key,&retained);
    if(result>=0) result=nv_cache_lookup(&core->cache,key,&retained);
    if(result==1) {
        copy=malloc(strlen(retained)+1);
        if(!copy) result=-1;
        else {strcpy(copy,retained);*value=copy;}
    } else if(!result) *value=NULL;
    unlock(core);return result;
}
int nv_core_change_ack(struct nv_core *core,const char *key,const char *value,int unset) {
    size_t length;int result;
    if(nv_key_length(key,&length)) return -1;
    if(lock(core)) return -1;
    /* Even a timeout can follow an applied update. Do not retain a clean entry. */
    nv_cache_invalidate(&core->cache,key);
    result=nv_client_change_ack(core->exchange,core->context,0,key,value,unset);
    unlock(core);return result;
}
int nv_core_getall(struct nv_core *core,void *output,size_t capacity,size_t *used) {
    int result;if(lock(core)) return -1;
    result=nv_getall_pages(core->exchange,core->context,0,output,capacity,used);
    unlock(core);return result;
}
static void invalidate(void *opaque,const char *key) {
    struct nv_core *core=opaque;nv_cache_invalidate(&core->cache,key);
}
int nv_core_notification(struct nv_core *core,const void *data,size_t size) {
    int result=0;if(lock(core)) return -1;
    if(data) result=nv_notification(data,size,invalidate,core);
    else nv_cache_invalidate(&core->cache,NULL);
    unlock(core);return result;
}
struct load_context {struct nv_core *core;size_t *confirmed;};
static int apply_verified(void *opaque,const char *key,const char *value) {
    struct load_context *load=opaque;struct nv_core *core=load->core;
    char *observed=NULL;int result,saved;
    nv_cache_invalidate(&core->cache,key);
    if(nv_client_change_ack(core->exchange,core->context,0,key,value,0)) return -1;
    result=nv_client_get(core->exchange,core->context,0,key,&observed);
    if(result<0) return -1;
    if(result!=1 || strcmp(observed,value)) {
        free(observed);errno=EIO;return -1;
    }
    saved=errno;free(observed);errno=saved;
    ++*load->confirmed;return 0;
}
int nv_core_load_verified(struct nv_core *core,FILE *input,size_t *confirmed) {
    struct load_context load={core,confirmed};int result;
    if(!input || !confirmed) {errno=EINVAL;return -1;}
    if(lock(core)) return -1;
    *confirmed=0;
    result=nv_settings_load(input,apply_verified,&load);
    unlock(core);return result;
}
int nv_core_save_at(struct nv_core *core,int directory,size_t capacity) {
    unsigned char *data;size_t used=0;int result,saved;
    if(directory<0 || !capacity || capacity>INT_MAX) {errno=EINVAL;return -1;}
    if(lock(core)) return -1;
    data=malloc(capacity);
    if(!data) {unlock(core);return -1;}
    result=nv_getall_pages(core->exchange,core->context,0,data,capacity,&used);
    if(!result) {
        if(!used || !data[0]) {errno=ENODATA;result=-1;}
        else result=nv_persist_at(directory,data,used);
    }
    saved=errno;free(data);errno=saved;unlock(core);return result;
}
