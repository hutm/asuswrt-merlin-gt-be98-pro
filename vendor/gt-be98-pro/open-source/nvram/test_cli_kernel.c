/* SPDX-License-Identifier: GPL-2.0-only */
#define _POSIX_C_SOURCE 200809L
#include "cli_kernel.h"
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
static char events[512];static int used,fail;static const char *stop;
static void event(const char *op,const char *key,const char *value){used+=snprintf(events+used,sizeof events-used,"%s:%s:%s;",op,key,value?value:"<NULL>");assert(used<(int)sizeof events);}
char *nvram_get(const char *key){if(!strcmp(key,"asus_stop_commit"))return (char *)stop;assert(!strcmp(key,"key"));return "value";}
int nvram_set(const char *key,const char *value){event("A",key,value);if(fail){errno=EIO;return -1;}return 3;}
int nvram_unset(const char *key){event("U",key,NULL);return 0;}
int nvram_kset(const char *key,const char *value){event("K",key,value);return 0;}
int wlcsm_nvram_set(const char *key,const char *value){event("W",key,value);return 0;}
int nvram_kcommit(void){event("C","",NULL);return 0;}
char *wlcsm_nvram_get(const char *key){(void)key;return NULL;}
char *wlcsm_nvram_get_bitflag(const char *key,unsigned bit){(void)key;(void)bit;return NULL;}
int wlcsm_nvram_set_bitflag(const char *key,unsigned bit,int enabled){(void)key;(void)bit;(void)enabled;return 0;}
static void reset(void){used=0;events[0]=0;}
int main(void){
 FILE *out=tmpfile();assert(out);char *set[]={"kset","key=a=b"};assert(nv_cli_kernel(2,set,out)==2&&!strcmp(events,"A:key:a=b;K:key:a=b;"));
 reset();set[1]="key";assert(nv_cli_kernel(2,set,out)==2&&!strcmp(events,"A:key:<NULL>;K:key:<NULL>;"));
 reset();set[0]="kunset";assert(nv_cli_kernel(2,set,out)==2&&!strcmp(events,"U:key:<NULL>;K:key:<NULL>;"));
 reset();set[0]="kset";fail=1;assert(nv_cli_kernel(2,set,out)==-1&&errno==EIO&&!strcmp(events,"A:key:<NULL>;"));fail=0;
 set[0]="kget";assert(nv_cli_kernel(2,set,out)==2);rewind(out);char text[32]={0};assert(fread(text,1,sizeof text,out)==6&&!strcmp(text,"value\n"));
 reset();set[0]="kcommit";stop="0";assert(nv_cli_kernel(1,set,out)==1&&!used);stop=NULL;assert(nv_cli_kernel(1,set,out)==1&&!strcmp(events,"C::<NULL>;"));
 char path[]="/tmp/cli-kernel-XXXXXX";int fd=mkstemp(path);assert(fd>=0);const char data[]="#ignored=x\n key =one\nkey=two\nempty=\n";assert(write(fd,data,sizeof data-1)==sizeof data-1);close(fd);
 reset();set[0]="kernelset";set[1]=path;assert(nv_cli_kernel(2,set,out)==2&&!strcmp(events,"K:key:one;K:key:two;K:empty:;"));reset();set[0]="loadfile";assert(nv_cli_kernel(2,set,out)==2&&!strcmp(events,"W:key:one;W:key:two;W:empty:;"));set[0]="kernelset";unlink(path);
 assert(nv_cli_kernel(2,set,out)==-1&&errno==ENOENT);assert(nv_cli_kernel(1,set,out)==-1&&errno==EINVAL);
 set[0]="unknown";assert(!nv_cli_kernel(1,set,out));fclose(out);
 puts("Kernel CLI: routing order, NULL values, stop control, file loading and failures passed");return 0;
}
