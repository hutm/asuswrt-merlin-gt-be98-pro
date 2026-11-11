/* SPDX-License-Identifier: GPL-2.0-only */
#define _POSIX_C_SOURCE 200809L
#include "access.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
static const char *executable;static unsigned reads;
ssize_t readlink(const char *path,char *out,size_t size){assert(!strcmp(path,"/proc/self/exe"));++reads;if(!executable){errno=ENOENT;return -1;}size_t n=strlen(executable);if(n>size)n=size;memcpy(out,executable,n);return (ssize_t)n;}
void dbg(const char *format,...){(void)format;}
#ifdef REFERENCE
#include <dlfcn.h>
#endif
int main(void){
#ifdef REFERENCE
 assert(dlopen("libnvram.so",RTLD_LAZY|RTLD_GLOBAL));void *h=dlopen("libshared.so",RTLD_NOW);assert(h);
 struct INVALID_NVRAM_GET_TABLE *table=dlsym(h,"invalid_nvram_get_t");
 int (*name_check)(char *)=dlsym(h,"invalid_nvram_get_name"),(*program)(char *)=dlsym(h,"invalid_nvram_get_program");
 int (*check)(void)=dlsym(h,"invalid_program_check");assert(table&&name_check&&program&&check);
#else
 struct INVALID_NVRAM_GET_TABLE *table=invalid_nvram_get_t;
 int (*name_check)(char *)=invalid_nvram_get_name,(*program)(char *)=invalid_nvram_get_program;
 int (*check)(void)=invalid_program_check;
#endif
 const char *paths[]={"/usr/bin/nvram","/bin/busybox","/sbin/rc","/tmp/app","/jffs/app","/usr/local/bin/app","/usr/","/bin/","/sbin/","/usr","/bin-extra/app","relative","","/opt/bin/app"};
 unsigned count=0;
 for(;table[count].name;++count){assert(!table[count].exclude_process);printf("T %s\n",table[count].name);}
 assert(count==20);
 for(unsigned p=0;p<14;++p){
  executable=paths[p];printf("P %u %d\n",p,check());
  for(unsigned n=0;n<count+3;++n){
   char near[128];char *name=n<count?table[n].name:n==count?NULL:n==count+1?"ordinary":"http_passwd_suffix";
   reads=0;int blocked=program(name);printf("N %u %u %d %d %u\n",p,n,name_check(name),blocked,reads);
   if(n<count){snprintf(near,sizeof near,"%s_suffix",name);assert(!name_check(near)&&!program(near));}
  }
 }
#ifndef REFERENCE
 executable=NULL;assert(check()==1&&program("http_passwd")==1);
 char long_path[140];memset(long_path,'a',sizeof long_path);memcpy(long_path,"/usr/",5);long_path[139]=0;
 executable=long_path;assert(check()==1);assert(program("ordinary")==0);
#endif
 return 0;
}
