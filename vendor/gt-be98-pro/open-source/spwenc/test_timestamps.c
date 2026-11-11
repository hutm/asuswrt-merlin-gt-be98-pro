/* SPDX-License-Identifier: GPL-2.0-only */
#include "timestamps.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
static char *eula,*policy;static int ready;
int nvram_get_int(const char *name){assert(!strcmp(name,"ntp_ready"));return ready;}
char *nvram_get(const char *name){if(!strcmp(name,"ASUS_NEW_EULA_ts"))return eula;assert(!strcmp(name,"537632ee"));return policy;}
int nvram_set(const char *name,const char *value){printf("W %s %s\n",name,value);return 0;}
char *rfctime(const time_t *t,char *out,int capacity){assert(capacity==100);snprintf(out,(size_t)capacity,"time:%ld",(long)*t);return out;}
#ifdef REFERENCE
#include <dlfcn.h>
#endif
int main(void){
#ifdef REFERENCE
 assert(dlopen("libnvram.so",RTLD_LAZY|RTLD_GLOBAL));void *h=dlopen("libshared.so",RTLD_NOW);assert(h);
 void (*update)(time_t,int)=dlsym(h,"update_ntp_ts");assert(update);
#else
 void (*update)(time_t,int)=update_ntp_ts;
#endif
 char *values[]={NULL,"","0","1","99","100","101","-1","invalid","  +42x","2147483647","2147483648","4294967295","9999999999","-999999999","123456789012345"};
 int before[]={0,100,2147483647};int delta[]={-200,0,200,2147483647};
 for(ready=0;ready<3;++ready)for(unsigned v=0;v<16;++v)for(unsigned b=0;b<3;++b)for(unsigned d=0;d<4;++d){
  eula=values[v];policy=values[(v+5)%16];printf("C %d %u %u %u\n",ready,v,b,d);update(before[b],delta[d]);
 }
 return 0;
}
