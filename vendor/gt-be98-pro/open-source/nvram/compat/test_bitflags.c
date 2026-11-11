/* SPDX-License-Identifier: GPL-2.0-only */
#define _GNU_SOURCE
#include <assert.h>
#include <dlfcn.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
int invalid_program_check(void) { return 0; }
int psname(void) { return 0; }
int logmessage_normal(void) { return 0; }
int jffs_nvram_get(void) { return 0; }
int large_nvram(void) { return 0; }
int jffs_nvram_unset(void) { return 0; }
int invalid_nvram_get_program(void) { return 0; }
int jffs_nvram_set(void) { return 0; }
int jffs_nvram_getall(void) { return 0; }
static const char *current;static int calls;static char written[64];
char *wlcsm_nvram_get(const char *key){assert(!strcmp(key,"key"));return (char *)current;}
int wlcsm_nvram_set(const char *key,const char *value){assert(!strcmp(key,"key"));++calls;snprintf(written,sizeof written,"%s",value);return 17;}
int main(int argc,char **argv){
 assert(argc==2);void *lib=dlopen(argv[1],RTLD_NOW);if(!lib){puts(dlerror());return 2;}
 char *(*get)(const char *,unsigned)=dlsym(lib,"wlcsm_nvram_get_bitflag");
 int (*set)(const char *,unsigned,int)=dlsym(lib,"wlcsm_nvram_set_bitflag");assert(get&&set);
 char *(*alias_get)(const char *,unsigned)=dlsym(lib,"nvram_get_bitflag");
 int (*alias_set)(const char *,unsigned,int)=dlsym(lib,"nvram_set_bitflag");assert(alias_get&&alias_set);
 const char *cases[]={NULL,"","0","1","80000000","ffffffff","junk","-2","100000000","0x12tail","-ffffffff","-100000000","  +ab  ","--1","- 1","+0x","ffffffffffffffffffffffff", "0000000080000000"};
 unsigned bits[34];for(unsigned i=0;i<33;++i)bits[i]=i;bits[33]=~0u;
 for(size_t i=0;i<sizeof cases/sizeof *cases;++i)for(size_t b=0;b<sizeof bits/sizeof *bits;++b){
  current=cases[i];char *v=get("key",bits[b]);printf("get %zu %u %s\n",i,bits[b],v?v:"NULL");
  char *alias=alias_get("key",bits[b]);assert((!v&&!alias)||(v&&alias&&!strcmp(v,alias)));
  for(int on=0;on<=1;++on){
   calls=0;written[0]=0;int r=set("key",bits[b],on);
   printf("set %zu %u %d %d %d %s\n",i,bits[b],on,r,calls,written);
   char expected[64];strcpy(expected,written);int expected_calls=calls;
   calls=0;written[0]=0;assert(alias_set("key",bits[b],on)==r);
   assert(calls==expected_calls&&!strcmp(written,expected));
  }
 }
 return 0;
}
