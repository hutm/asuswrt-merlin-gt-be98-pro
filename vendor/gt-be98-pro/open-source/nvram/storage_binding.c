/* SPDX-License-Identifier: GPL-2.0-only */
#define _POSIX_C_SOURCE 200809L
#include "storage_binding.h"
#include "storage_lock.h"
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
extern int large_nvram(const char *);
extern char *jffs_nvram_get(const char *);
extern int jffs_nvram_set(const char *,const char *);
extern int jffs_nvram_unset(const char *);
extern int jffs_nvram_getall(int,char *,int);
extern int invalid_nvram_get_program(char *);
extern int invalid_program_check(void);
static int acquire(void *opaque) {
    struct nv_storage_binding *b=opaque;int error=pthread_mutex_lock(&b->mutex);
    if(error) {errno=error;return -1;}
    b->owned_fd=nv_storage_lock_acquire(b->directory,10,500);
    if(b->owned_fd<0) {int saved=errno;pthread_mutex_unlock(&b->mutex);errno=saved;return -1;}
    return 0;
}
static int release(void *opaque) {
    struct nv_storage_binding *b=opaque;
    int result=nv_storage_lock_release(b->directory,b->owned_fd),saved=errno;
    b->owned_fd=-1;pthread_mutex_unlock(&b->mutex);errno=saved;return result;
}
static int classify(void *context,const char *key) {(void)context;return large_nvram(key);}
static char *get(void *context,const char *key) {(void)context;return jffs_nvram_get(key);}
static int set(void *context,const char *key,const char *value) {(void)context;return jffs_nvram_set(key,value);}
static int unset(void *context,const char *key) {(void)context;return jffs_nvram_unset(key);}
static int getall(void *context,int used,char *output,int capacity) {(void)context;return jffs_nvram_getall(used,output,capacity);}
static int deny_get(void *context,const char *key) {(void)context;return invalid_nvram_get_program((char *)key);}
static int deny_all(void *context) {(void)context;return invalid_program_check();}
int nv_storage_binding_init(struct nv_storage_binding *b,int directory,struct nv_storage_hooks *hooks) {
    int fd,error;
    if(!b || !hooks || directory<0) {errno=EINVAL;return -1;}
    fd=fcntl(directory,F_DUPFD_CLOEXEC,0);if(fd<0) return -1;
    error=pthread_mutex_init(&b->mutex,NULL);
    if(error) {close(fd);errno=error;return -1;}
    b->directory=fd;b->owned_fd=-1;
    *hooks=(struct nv_storage_hooks){b,classify,acquire,release,get,set,unset,getall,deny_get,deny_all};
    return 0;
}
int nv_storage_binding_destroy(struct nv_storage_binding *b) {
    int error;
    if(!b || b->directory<0) {errno=EINVAL;return -1;}
    if(b->owned_fd>=0) {errno=EBUSY;return -1;}
    error=pthread_mutex_destroy(&b->mutex);
    if(error) {errno=error;return -1;}
    int result=close(b->directory);b->directory=-1;return result;
}
