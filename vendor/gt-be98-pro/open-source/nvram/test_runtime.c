/* SPDX-License-Identifier: GPL-2.0-only */
#define _POSIX_C_SOURCE 200809L
#include "runtime.h"
#include "frame.h"
#include "pair.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
static char path[]="/tmp/nv-runtime-XXXXXX",small[64],large[64];
static int present,deny,fail_init=1,requests,reenter;
int nv_runtime_test_directory(void){if(fail_init){fail_init=0;errno=ENOENT;return -1;}return open(path,O_RDONLY|O_DIRECTORY);}
int nv_runtime_test_exchange(void *context,const void *request,size_t size,void *reply,size_t capacity,size_t *returned){
 (void)context;struct nv_frame_view f;struct nv_pair_view p;unsigned char payload[1024];size_t n=0;
 assert(!nv_frame_decode(request,size,&f));++requests;
 if(reenter){assert(!nvram_get("key")&&errno==EDEADLK);}
 if(f.type==3){assert(!strcmp((const char *)f.payload,"key"));if(present)assert(!nv_pair_encode(payload,sizeof payload,"key",small,strlen(small)+1,&n));}
 else if(f.type==2||f.type==4){assert(!nv_pair_decode(f.payload,f.length,&p)&&!strcmp(p.name,"key"));present=f.type==2;if(present){assert(p.value_len<=sizeof small);memcpy(small,p.value,p.value_len);}memcpy(payload,f.payload,f.length);n=f.length;}
 else {assert(f.type==5);memcpy(payload,"key=one",8);n=8;}
 return nv_frame_encode(reply,capacity,f.type==5?6:f.type,0,0,payload,n,returned);
}
int large_nvram(const char *key){return !strcmp(key,"large");}
char *jffs_nvram_get(const char *key){assert(!strcmp(key,"large"));return large;}
int jffs_nvram_set(const char *key,const char *value){assert(!strcmp(key,"large"));if(!value){*large=0;return 0;}assert(strlen(value)<sizeof large);strcpy(large,value);return strlen(value);}
int jffs_nvram_unset(const char *key){return jffs_nvram_set(key,NULL);}
int jffs_nvram_getall(int used,char *out,int capacity){assert(used==8);assert(capacity>used+10);memcpy(out+used,"large=two",10);return used+10;}
int invalid_nvram_get_program(char *key){(void)key;return deny;}
int invalid_program_check(void){return deny;}
static void *worker(void *unused){(void)unused;for(int i=0;i<100;++i){char out[64];assert(!nvram_set("key","parallel"));assert(nvram_getall(out,sizeof out)==18);}return NULL;}
int main(void){
 assert(mkdtemp(path));assert(!nvram_get("key")&&errno==ENOENT);assert(!nvram_get("key"));
 assert(!nvram_set("key","one"));char *first=nvram_get("key");assert(first&&!strcmp(first,"one"));
 assert(!wlcsm_nvram_set("key","two"));assert(wlcsm_nvram_get("key")==first&&!strcmp(first,"two"));
 int before=requests;assert(nvram_set("large","big")==3&&!strcmp(nvram_get("large"),"big")&&requests==before);
 assert(!nvram_unset("large")&&!*large);deny=1;before=requests;assert(!nvram_get("key")&&errno==EACCES&&requests==before);assert(!strcmp(wlcsm_nvram_get("key"),"two"));deny=0;
 char out[64];assert(nvram_getall(out,sizeof out)==18&&!memcmp(out,"key=one\0large=two\0",18));
 memset(out,0x55,sizeof out);assert(wlcsm_nvram_getall(out,sizeof out)==8&&!memcmp(out,"key=one\0",8));for(int i=8;i<64;++i)assert(!out[i]);
 assert(!nvram_set("key",NULL)&&!nvram_get("key"));assert(!wlcsm_nvram_unset("key"));
 reenter=1;assert(!nvram_set("key","guarded"));reenter=0;
 pthread_t threads[4];for(int i=0;i<4;++i)assert(!pthread_create(&threads[i],NULL,worker,NULL));for(int i=0;i<4;++i)assert(!pthread_join(threads[i],NULL));
 pid_t child=fork();assert(child>=0);if(!child){assert(!strcmp(nvram_get("key"),"parallel"));assert(nvram_set("large","child")==5);_exit(0);}
 int status;assert(waitpid(child,&status,0)==child&&WIFEXITED(status)&&WEXITSTATUS(status)==0);
 assert(!strcmp(nvram_get("key"),"parallel"));assert(nvram_set("large","parent")==6);
 assert(!rmdir(path));puts("NVRAM runtime: exported APIs, initialization retry, storage policy/locks, reentry guard, threads and post-fork use passed");return 0;
}
