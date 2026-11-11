/* SPDX-License-Identifier: GPL-2.0-only */
#include "adapter.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
int nv_adapter_init(struct nv_adapter *a,struct nv_core *core,const struct nv_storage_hooks *h) {
    if(!a || !core || !h || !h->is_large || !h->lock || !h->unlock || !h->get ||
       !h->set || !h->unset || !h->getall || !h->deny_get || !h->deny_getall) {
        errno=EINVAL;return -1;
    }
    a->core=core;a->storage=*h;return 0;
}
static int valid(struct nv_adapter *a,const char *key) {
    if(!a || !a->core || !key) {errno=EINVAL;return 0;}return 1;
}
static void unlock(struct nv_adapter *a) {
    int saved=errno;a->storage.unlock(a->storage.context);errno=saved;
}
char *nv_adapter_get(struct nv_adapter *a,const char *key) {
    char *value=NULL;
    if(!valid(a,key)) return NULL;
    if(a->storage.deny_get(a->storage.context,key)) {errno=EACCES;return NULL;}
    if(!a->storage.is_large(a->storage.context,key)) {
        if(nv_core_get_borrowed(a->core,key,&value)<0) return NULL;
        return value;
    }
    if(a->storage.lock(a->storage.context)) return NULL;
    value=a->storage.get(a->storage.context,key);unlock(a);return value;
}
int nv_adapter_set(struct nv_adapter *a,const char *key,const char *value) {
    int result;
    if(!valid(a,key)) return -1;
    if(!a->storage.is_large(a->storage.context,key))
        return nv_core_change_ack(a->core,key,value,value==NULL);
    if(a->storage.lock(a->storage.context)) return -1;
    result=a->storage.set(a->storage.context,key,value);unlock(a);return result;
}
int nv_adapter_unset(struct nv_adapter *a,const char *key) {
    int result;
    if(!valid(a,key)) return -1;
    if(!a->storage.is_large(a->storage.context,key))
        return nv_core_change_ack(a->core,key,NULL,1);
    if(a->storage.lock(a->storage.context)) return -1;
    result=a->storage.unset(a->storage.context,key);unlock(a);return result;
}
int nv_adapter_getall(struct nv_adapter *a,char *output,int capacity) {
    char *scratch;size_t used=0;int result,saved;
    if(!a || !a->core || !output || capacity<=0) {errno=EINVAL;return -1;}
    if(a->storage.deny_getall(a->storage.context)) {errno=EACCES;return -1;}
    scratch=calloc(1,(size_t)capacity);if(!scratch) return -1;
    result=nv_core_getall(a->core,scratch,(size_t)capacity,&used);
    if(!result && used>=(size_t)capacity) {errno=ENOSPC;result=-1;}
    if(!result) {
        if(a->storage.lock(a->storage.context)) result=-1;
        else {
            result=a->storage.getall(a->storage.context,(int)used,scratch,capacity);
            unlock(a);
            if(result>=0 && (result<(int)used || result>=capacity)) {errno=EPROTO;result=-1;}
        }
    }
    if(result>=0) memcpy(output,scratch,(size_t)capacity);
    saved=errno;free(scratch);errno=saved;return result;
}
