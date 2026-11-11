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
#include <stdint.h>
#include <errno.h>
int main(int argc,char **argv){
 assert(argc==2);void *lib=dlopen(argv[1],RTLD_NOW);if(!lib){puts(dlerror());return 2;}
 int (*parse)(const char *,int,int32_t *)=dlsym(lib,"wlcsm_strtol");assert(parse);
 const char *cases[]={NULL,"","0","-0","1","-1","2147483647","2147483648","-2147483648","-2147483649","0x80000000","0xffffffff","99999999999999999999999999999","-99999999999999999999999999999"," +12tail","word","08","--1","  ","0x"};
 int bases[]={0,2,8,10,16,36,37};
 for(size_t i=0;i<sizeof cases/sizeof *cases;++i)for(size_t b=0;b<sizeof bases/sizeof *bases;++b){
  int32_t value=12345;errno=EIO;int result=parse(cases[i],bases[b],&value);
  printf("%zu %d %d %d %d\n",i,bases[b],result,(int)value,errno);
 }
 return 0;
}
