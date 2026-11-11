/* SPDX-License-Identifier: GPL-2.0-only */
#define _POSIX_C_SOURCE 200809L
#include "storage_binding.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
int large_nvram(const char *key){return !strcmp(key,"large");}
char *jffs_nvram_get(const char *key){assert(!strcmp(key,"large"));return "value";}
int jffs_nvram_set(const char *key,const char *value){assert(!strcmp(key,"large")&&!strcmp(value,"value"));return 5;}
int jffs_nvram_unset(const char *key){assert(!strcmp(key,"large"));return 0;}
int jffs_nvram_getall(int used,char *out,int capacity){assert(used==3&&capacity==64);memcpy(out+used,"large=value",12);return used+12;}
int invalid_nvram_get_program(char *key){return !strcmp(key,"denied");}
int invalid_program_check(void){return 7;}
int main(void){
 char path[]="/tmp/nv-bind-XXXXXX",out[64]={0};struct nv_storage_binding b;struct nv_storage_hooks h;
 assert(mkdtemp(path));int directory=open(path,O_RDONLY|O_DIRECTORY);assert(directory>=0);
 assert(!nv_storage_binding_init(&b,directory,&h));close(directory);
 assert(h.is_large(h.context,"large")&&!h.is_large(h.context,"small"));
 assert(h.deny_get(h.context,"denied")&&!h.deny_get(h.context,"large")&&h.deny_getall(h.context)==7);
 assert(!h.lock(h.context));assert(nv_storage_binding_destroy(&b)==-1&&errno==EBUSY);
 assert(!strcmp(h.get(h.context,"large"),"value"));assert(h.set(h.context,"large","value")==5);
 assert(!h.unset(h.context,"large"));assert(h.getall(h.context,3,out,64)==15&&!strcmp(out+3,"large=value"));
 assert(!h.unlock(h.context));assert(faccessat(b.directory,"nvram.lock",F_OK,0)==-1&&errno==ENOENT);
 assert(!h.lock(h.context));assert(!unlinkat(b.directory,"nvram.lock",0));
 int replacement=openat(b.directory,"nvram.lock",O_CREAT|O_EXCL|O_WRONLY,0644);assert(replacement>=0);close(replacement);
 assert(h.unlock(h.context)==-1&&errno==ESTALE);assert(!unlinkat(b.directory,"nvram.lock",0));
 assert(!h.lock(h.context)&&!h.unlock(h.context));
 assert(!nv_storage_binding_destroy(&b));assert(!rmdir(path));
 puts("Storage binding: real lock lifecycle, shared-hook forwarding, policy routing and release failure passed");return 0;
}
