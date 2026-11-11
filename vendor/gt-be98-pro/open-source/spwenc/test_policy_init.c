/* SPDX-License-Identifier: GPL-2.0-only */
#include "policy.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static char *eula,*policy;static int eula_ver,policy_ver;
char *nvram_get(const char *name){if(!strcmp(name,"ASUS_NEW_EULA_ts"))return eula;assert(!strcmp(name,"537632ee"));return policy;}
int nvram_get_int(const char *name){if(!strcmp(name,"ASUS_NEW_EULA"))return eula_ver;assert(!strcmp(name,"68feaaa3"));return policy_ver;}
int nvram_set(const char *name,const char *value){printf("W %s %s\n",name,value);return 0;}
#ifdef REFERENCE
#include <dlfcn.h>
#endif
int main(void){
#ifdef REFERENCE
 assert(dlopen("libnvram.so",RTLD_LAZY|RTLD_GLOBAL));void *h=dlopen("libshared.so",RTLD_NOW);assert(h);void (*init)(void)=dlsym(h,"init_asus_pp_eula");assert(init);
#else
 void (*init)(void)=init_asus_pp_eula;
#endif
 char *values[]={NULL,"","0","123","000123","-1","+1"," 123","123 ","1x","<1>123","<123",">123","abc<>xyz","999999999999999999999","1.5"};
 int versions[]={0,1,2,5,-1,12345678,-2147483647-1};
 for(unsigned v=0;v<16;++v)for(unsigned n=0;n<7;++n){
  eula=values[v];policy=values[(v+5)%16];eula_ver=versions[n];policy_ver=versions[(n+3)%7];printf("C %u %u\n",v,n);init();
 }
 char large[1101];
 unsigned lengths[]={1010,1013,1014,1020,1023,1024,1100};
 for(unsigned n=0;n<7;++n){
  memset(large,'1',lengths[n]);large[lengths[n]]=0;eula=policy=large;eula_ver=12345678;policy_ver=-12345678;printf("L %u\n",lengths[n]);init();
 }
 return 0;
}
