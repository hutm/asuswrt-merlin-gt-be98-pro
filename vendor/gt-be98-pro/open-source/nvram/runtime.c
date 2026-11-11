/* SPDX-License-Identifier: GPL-2.0-only */
#define _POSIX_C_SOURCE 200809L
#include "runtime.h"
#include "storage_binding.h"
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#ifndef NV_RUNTIME_TEST
#include "transport.h"
static struct nv_transport transport={31,5000,0,1};
#else
extern int nv_runtime_test_exchange(void *,const void *,size_t,void *,size_t,size_t *);
extern int nv_runtime_test_directory(void);
extern int nv_runtime_test_data_directory(void);
#endif
static pthread_mutex_t runtime_mutex=PTHREAD_MUTEX_INITIALIZER;
static struct nv_core *core;
static struct nv_adapter adapter;
static struct nv_storage_binding binding;
static int fork_error;
static _Thread_local int in_call;
static void fork_prepare(void) {pthread_mutex_lock(&runtime_mutex);}
static void fork_finish(void) {pthread_mutex_unlock(&runtime_mutex);}
__attribute__((constructor)) static void register_fork_handlers(void) {
    fork_error=pthread_atfork(fork_prepare,fork_finish,fork_finish);
}
/* The outer mutex excludes all runtime operations during fork, so neither core
 * nor binding mutex is inherited while held. Forking from a callback inside an
 * operation is unsupported. Borrowed pointers retain the normal process copy. */
static int initialize(void) {
    struct nv_storage_hooks hooks;struct nv_core *candidate;int directory,saved;
    if(core) return 0;
#ifdef NV_RUNTIME_TEST
    directory=nv_runtime_test_directory();
    if(directory<0) return -1;
    candidate=nv_core_create(nv_runtime_test_exchange,NULL,4*1024*1024);
#else
    directory=open("/var",O_RDONLY|O_DIRECTORY|O_CLOEXEC);
    if(directory<0) return -1;
    candidate=nv_core_create(nv_socket_exchange,&transport,4*1024*1024);
#endif
    if(!candidate) {saved=errno;close(directory);errno=saved;return -1;}
    if(nv_storage_binding_init(&binding,directory,&hooks)) {
        saved=errno;close(directory);nv_core_destroy(candidate);errno=saved;return -1;
    }
    close(directory);
    if(nv_adapter_init(&adapter,candidate,&hooks)) {
        saved=errno;nv_storage_binding_destroy(&binding);nv_core_destroy(candidate);errno=saved;return -1;
    }
    core=candidate;return 0;
}
static int enter(void) {
    int error;
    if(in_call) {errno=EDEADLK;return -1;}
    if(fork_error) {errno=fork_error;return -1;}
    error=pthread_mutex_lock(&runtime_mutex);if(error) {errno=error;return -1;}
    in_call=1;
    if(initialize()) {int saved=errno;in_call=0;pthread_mutex_unlock(&runtime_mutex);errno=saved;return -1;}
    return 0;
}
static void leave(void) {
    int saved=errno;in_call=0;pthread_mutex_unlock(&runtime_mutex);errno=saved;
}
char *nvram_get(const char *key) {char *value;if(enter()) return NULL;value=nv_adapter_get(&adapter,key);leave();return value;}
int nvram_set(const char *key,const char *value) {int r;if(enter()) return -1;r=nv_adapter_set(&adapter,key,value);leave();return r;}
int nvram_unset(const char *key) {int r;if(enter()) return -1;r=nv_adapter_unset(&adapter,key);leave();return r;}
int nvram_getall(char *out,int capacity) {int r;if(enter()) return -1;r=nv_adapter_getall(&adapter,out,capacity);leave();return r;}
char *wlcsm_nvram_get(const char *key) {
    char *value=NULL;if(enter()) return NULL;
    if(nv_core_get_borrowed(core,key,&value)<0) value=NULL;
    leave();return value;
}
int wlcsm_nvram_set(const char *key,const char *value) {
    int r;if(enter()) return -1;r=nv_core_change_ack(core,key,value,value==NULL);leave();return r;
}
int wlcsm_nvram_unset(const char *key) {
    int r;if(enter()) return -1;r=nv_core_change_ack(core,key,NULL,1);leave();return r;
}
int wlcsm_nvram_getall(char *output,int capacity) {
    char *scratch;size_t used=0;int result,saved;
    if(!output || capacity<=0) {errno=EINVAL;return -1;}
    if(enter()) return -1;
    scratch=calloc(1,(size_t)capacity);if(!scratch) {leave();return -1;}
    result=nv_core_getall(core,scratch,(size_t)capacity,&used);
    if(!result && used>=(size_t)capacity) {errno=ENOSPC;result=-1;}
    if(!result) {memcpy(output,scratch,(size_t)capacity);result=(int)used;}
    saved=errno;free(scratch);errno=saved;leave();return result;
}

/* Serialize source writers with the same exclusive lock used for JFFS storage.
 * Original binaries do not necessarily take this lock when committing; mixing
 * implementations cannot provide cross-process commit serialization. */
int wlcsm_nvram_commit(void) {
    int directory,result,saved,released;
    struct nv_storage_hooks *storage=&adapter.storage;
    if(enter()) return 1;
    if(storage->lock(storage->context)) {leave();return 1;}
#ifdef NV_RUNTIME_TEST
    directory=nv_runtime_test_data_directory();
#else
    directory=open("/data",O_RDONLY|O_DIRECTORY|O_CLOEXEC);
#endif
    if(directory<0) result=-1;
    else {
        result=nv_core_commit_at(core,directory,262144);
        saved=errno;
        if(close(directory) && !result) result=-1;
        else errno=saved;
    }
    saved=errno;
    released=storage->unlock(storage->context);
    if(result) errno=saved;
    else if(released) result=-1;
    leave();return result?1:0;
}
