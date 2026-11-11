/* SPDX-License-Identifier: GPL-2.0-only */
#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 200809L
#include "runtime.h"
#include "storage_binding.h"
#include "manager.h"
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
extern int nv_runtime_test_manager_exchange(const void *,size_t,int (*)(void *,const void *,size_t),void *);
extern int nv_runtime_test_directory(void);
extern int nv_runtime_test_data_directory(void);
extern void nv_runtime_test_sync(void);
extern FILE *nv_runtime_test_commit_marker(void);
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
static int commit_locked(int notify) {
    int directory,result,saved,released;
    struct nv_storage_hooks *storage=&adapter.storage;
    if(storage->lock(storage->context)) return 1;
#ifdef NV_RUNTIME_TEST
    directory=nv_runtime_test_data_directory();
#else
    directory=open("/data",O_RDONLY|O_DIRECTORY|O_CLOEXEC);
#endif
    if(directory<0) result=-1;
    else {
        result=notify?nv_core_commit_at(core,directory,262144):
                      nv_core_save_at(core,directory,262144);
        saved=errno;
        if(close(directory) && !result) result=-1;
        else errno=saved;
    }
    saved=errno;
    released=storage->unlock(storage->context);
    if(result) errno=saved;
    else if(released) result=-1;
    return result?1:0;
}

int wlcsm_nvram_commit(void) {
    int result;if(enter()) return 1;
    result=commit_locked(1);leave();return result;
}
int nvram_commit(void) {
    char *stop=NULL;FILE *marker;int result,saved;
    if(enter()) return 1;
    /* Refuse to write if the control cannot be read. Any present value,
     * including empty or "0", suppresses the operation in the vendor ABI. */
    if(adapter.storage.deny_get(adapter.storage.context,"asus_stop_commit")) {
        errno=EACCES;leave();return 1;
    }
    result=nv_core_get_copy(core,"asus_stop_commit",&stop);
    if(result<0) {leave();return 1;}
    if(stop) {free(stop);leave();return 0;}
    result=commit_locked(1);saved=errno;
#ifdef NV_RUNTIME_TEST
    nv_runtime_test_sync();
#else
    sync();
#endif
    if(!result) {
#ifdef NV_RUNTIME_TEST
        marker=nv_runtime_test_commit_marker();
#else
        marker=fopen("/var/log/commit_ret","w");
#endif
        /* This is a best-effort status file, as in the reference. Failure
         * cannot undo the already successful settings transaction. */
        if(marker) {fputs("commit: OK\n",marker);fclose(marker);}
    }
    errno=saved;leave();return result;
}

char *nvram_get_salt(void) {
    char *value=NULL;if(enter()) return NULL;
    /* This ABI uses a distinct denied sentinel. Do not dereference it. */
    if(adapter.storage.deny_getall(adapter.storage.context)) {
        errno=EACCES;value=(char *)-1;
    } else if(nv_core_get_borrowed(core,"nvram_salt",&value)<0) value=NULL;
    leave();return value;
}

int common_nvram_kernel_set(const char *key,const char *value) {
    int result;if(enter()) return -1;
    /* This interface always sends SET, even for a NULL value. */
    result=nv_core_change_ack(core,key,value,0);leave();return result;
}
int nvram_kset(const char *key,const char *value) {
    return common_nvram_kernel_set(key,value);
}

int nvram_kcommit(void) {
    int result;if(enter()) return -1;
    /* Kernel snapshot only: no ASUS stop flag, global sync or notification.
     * Use the recovery-file transaction rather than truncating the live file. */
    result=commit_locked(0);leave();return result?-1:0;
}

char *wlcsm_nvram_xfr(const char *input) {
    char *value=NULL;if(enter()) return NULL;
    if(nv_core_xfr(core,input,&value)<0) value=NULL;
    leave();return value;
}
char *nvram_xfr(const char *input) {return wlcsm_nvram_xfr(input);}

static int restart_reply(void *state,const void *data,size_t size) {
    return nv_restart_accept(state,data,size);
}
int wlcsm_mngr_restart(unsigned radio,unsigned source,int persist,int wait) {
    unsigned char request[128];size_t size;struct nv_restart_reply state={0};int result;
    if(enter()) return 1;
    result=nv_restart_request(request,sizeof request,radio,source,persist,wait,&size);
    if(!result) {
#ifdef NV_RUNTIME_TEST
        result=nv_runtime_test_manager_exchange(request,size,restart_reply,&state);
#else
        unsigned char reply[1280];size_t received;
        result=nv_socket_exchange_until(&transport,request,size,reply,sizeof reply,&received,restart_reply,&state);
#endif
    }
    if(!result && !state.done) {errno=EPROTO;result=-1;}
    leave();return result?1:0;
}
