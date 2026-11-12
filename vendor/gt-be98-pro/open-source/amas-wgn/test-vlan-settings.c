#include "vlan.h"
#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
static const char *stored;static unsigned gets,sets,commits,cases;
char *nvram_get(const char *key){assert(!strcmp(key,"vlan_rulelist"));gets++;printf("get:%s;",key);return (char *)stored;}
int nvram_set(const char *key,const char *value){assert(!strcmp(key,"vlan_rulelist"));sets++;printf("set:%s:%s;",key,value);return 0;}
int nvram_commit(void){commits++;printf("commit;");return 0;}
int wgn_get_wl_band_unit(int *u,int *s,char *two,char *five){assert(!u&&!s);printf("band:%s:%s;",two,five);return 0;}
int wgn_guest_ifcount(int band){assert(!band);printf("limit;");return 1;}
int main(int argc,char **argv){assert(argc==2);void *h=dlopen(argv[1],RTLD_LAZY|RTLD_LOCAL);if(!h){puts(dlerror());return 1;}
 const char *names[]={"wgn_vlan_list_get_from_nvram","wgn_vlan_list_get_all_from_nvram","wgn_vlan_list_get_other_from_nvram"};
 const char *texts[]={NULL,"","<bad","<1>20>3>a>b>0002>0000>sub>1>0>1","<1>20>3>a>b>0002>0000>sub>1>0>0<1>21>3>a>b>0000>0002>sub>1>0>1","<>>>>>>>>>>2<>>>>>>>>>>1<>>>>>>>>>>-1"};
 for(unsigned n=0;n<3;n++)for(unsigned t=0;t<6;t++)for(size_t cap=0;cap<5;cap++)for(unsigned nulls=0;nulls<4;nulls++){
  wgn_vlan_rule *(*get)(wgn_vlan_rule *,size_t,size_t *)=dlsym(h,names[n]);assert(get);wgn_vlan_rule rules[4];memset(rules,0xa5,sizeof(rules));size_t count=99;stored=texts[t];gets=0;errno=0;
  wgn_vlan_rule *result=get(nulls&1?NULL:rules,cap,nulls&2?NULL:&count);
  printf("result:%d:%zu:%u:%d:",result==rules?1:result==NULL?0:-1,count,gets,errno);unsigned char *b=(void *)rules;for(size_t i=0;i<sizeof(rules);i++)if(b[i]!=0xa5)printf("%zu=%02x,",i,b[i]);puts("");cases++;
 }
 void (*set)(wgn_vlan_rule *,size_t)=dlsym(h,"wgn_vlan_list_set_to_nvram");assert(set);
 for(int previous=0;previous<=8;previous++)for(unsigned kind=0;kind<3;kind++)for(size_t count=0;count<4;count++)for(unsigned incoming=0;incoming<3;incoming++)for(int nulls=0;nulls<2;nulls++){
  char value[4000]="";unsigned retained=0;for(int r=0;r<previous;r++){char row[300];unsigned type=kind==2?(unsigned)r%2:kind;retained+=type!=1;snprintf(row,sizeof(row),"<1>%d>3>a>b>0002>0000>sub>1>0>%u",r,type);strcat(value,row);}
  if(retained+count>8)continue;
  stored=value;wgn_vlan_rule rules[3];memset(rules,0,sizeof(rules));for(unsigned r=0;r<3;r++){rules[r].enable=1;rules[r].vid=20+r;strcpy(rules[r].wl2gset,"0002");strcpy(rules[r].wl5gset,"0000");strcpy(rules[r].subnet_name,"192.168.1.1/24");rules[r].type=incoming==2?r%2:incoming;}
  gets=sets=commits=0;errno=0;set(nulls?NULL:rules,count);printf("write:%u:%u:%u:%d\n",gets,sets,commits,errno);cases++;
 }
 fprintf(stderr,"%u VLAN settings comparisons\n",cases);return 0;
}
