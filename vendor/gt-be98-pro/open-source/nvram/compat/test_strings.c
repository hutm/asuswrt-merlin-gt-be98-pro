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
static void dump(const unsigned char *p,size_t n){for(size_t i=0;i<n;++i)printf("%02x",p[i]);}
int main(int argc,char **argv){
 assert(argc==2);void *lib=dlopen(argv[1],RTLD_NOW);if(!lib){puts(dlerror());return 2;}
 char *(*trim[2])(char *)={dlsym(lib,"wlcsm_trim_str"),dlsym(lib,"wlcsm_trim_str2")};
 char *(*parse)(char *,int *,int *)=dlsym(lib,"wlcsm_nvram_name_parser");assert(trim[0]&&trim[1]&&parse);
 const char alphabet[]={' ',34,10,13,9,'a'};
 for(int which=0;which<2;++which){assert(trim[which](NULL)==NULL);
  unsigned cases=1;
  for(unsigned len=0;len<=5;++len){
   for(unsigned n=0;n<cases;++n){unsigned v=n;char data[8];memset(data,0x55,sizeof data);
    for(unsigned i=0;i<len;++i){data[i]=alphabet[v%6];v/=6;}data[len]=0;
    char *result=trim[which](data);assert(result>=data&&result<=data+len);
    printf("trim %d %u %u %ld ",which,len,n,(long)(result-data));dump((unsigned char *)data,sizeof data);putchar(10);
   }cases*=6;
  }
 }
 const char *names[]={NULL,"","_","wl0_ssid","wl1.2_key","wl-1.-2_x","wl12.9_","wl1junk_x","wl1._x","wl 2.3_x","wan0_x","wlx_x","wl1.2.3_x","wl0_foo_bar","wl+3.+4_x"};
 for(size_t i=0;i<sizeof names/sizeof *names;++i){int radio=77,subunit=88;char data[128];char *name=NULL;
  if(names[i]){strcpy(data,names[i]);name=data;}
  char *result=parse(name,&radio,&subunit);printf("parse %zu %d %d %ld %s\n",i,radio,subunit,result?(long)(result-name):-1,result?result:"NULL");
 }
 return 0;
}
