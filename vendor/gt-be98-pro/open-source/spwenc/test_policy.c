/* SPDX-License-Identifier: GPL-2.0-only */
#include "policy.h"
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static char *setting;static int gateway;static unsigned reads;
int webapi_get_b(int id,char *out,size_t size){assert(id==3&&size==32);strcpy(out,"policy_setting");return gateway;}
char *nvram_get(const char *name){assert(!strcmp(name,"policy_setting"));++reads;return setting;}
int nvram_get_int(const char *name){char *s=nvram_get(name);long n=s?strtol(s,NULL,10):0;return n>INT_MAX?INT_MAX:n<INT_MIN?INT_MIN:(int)n;}
#ifdef REFERENCE
#include <dlfcn.h>
#endif
int main(void){
#ifdef REFERENCE
 assert(dlopen("libnvram.so",RTLD_LAZY|RTLD_GLOBAL));void *h=dlopen("libshared.so",RTLD_NOW);assert(h);
 struct ASUS_PP_table *table=dlsym(h,"ASUS_PP_t");
 int (*state)(int)=dlsym(h,"get_ASUS_privacy_policy_state"),(*version)(int)=dlsym(h,"get_ASUS_privacy_policy_ver");
 int (*policy)(void)=dlsym(h,"get_ASUS_privacy_policy");assert(table&&state&&version&&policy);
#else
 struct ASUS_PP_table *table=ASUS_PP_t;
 int (*state)(int)=get_ASUS_privacy_policy_state,(*version)(int)=get_ASUS_privacy_policy_ver;
 int (*policy)(void)=get_ASUS_privacy_policy;
#endif
 unsigned n=0;for(;table[n].name;++n)printf("T %s %s %d %d\n",table[n].name,table[n].version,table[n].id,table[n].force_sign);assert(n==8);
 char *values[]={NULL,"","0","1","2","3","4","5","6","-1","  +4tail","invalid","99999999999999999999999999999999999999","-9999999999999999999999999999999999999","00000000000000000000000000000001"};
 for(gateway=-1;gateway<=2;++gateway)for(unsigned v=0;v<15;++v){
  setting=values[v];reads=0;int p=policy();assert(reads==(gateway>0?1U:0U));printf("P %d %u %d\n",gateway,v,p);
  for(int id=-1;id<=10;++id){reads=0;int s=state(id);assert(reads==(gateway>0?1U:0U));printf("S %d %u %d %d %d\n",gateway,v,id,s,version(id));}
 }
 return 0;
}
