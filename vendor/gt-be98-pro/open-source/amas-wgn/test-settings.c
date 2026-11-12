#include "subnet.h"
#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
static const char *stored;static unsigned gets,sets,commits,cases;
char *nvram_get(const char *key){assert(!strcmp(key,"subnet_rulelist"));gets++;printf("get:%s;",key);return (char *)stored;}
int nvram_set(const char *key,const char *value){assert(!strcmp(key,"subnet_rulelist"));sets++;printf("set:%s:%s;",key,value);return 0;}
int nvram_commit(void){commits++;printf("commit;");return 0;}
int main(int argc,char **argv){assert(argc==2);void *h=dlopen(argv[1],RTLD_LAZY|RTLD_LOCAL);if(!h){puts(dlerror());return 1;}
 const char *names[]={"wgn_subnet_list_get_from_nvram","wgn_subnet_list_get_all_from_nvram","wgn_subnet_list_get_other_from_nvram"};
 const char *texts[]={NULL,"","<bad","<a>b>1>c>d>2>e>f>g>3>h>1","<a>b>0>c>d>2>e>f>g>3>h>0<j>k>1>l>m>4>n>o>p>5>q>1","<>>>>>>>>>>>2<>>>>>>>>>>>1<>>>>>>>>>>>-1"};
 for(unsigned n=0;n<3;n++)for(unsigned t=0;t<6;t++)for(size_t cap=0;cap<5;cap++)for(unsigned nulls=0;nulls<4;nulls++){
  wgn_subnet_rule *(*get)(wgn_subnet_rule *,size_t,size_t *)=dlsym(h,names[n]);assert(get);wgn_subnet_rule rules[4];memset(rules,0xa5,sizeof(rules));size_t count=99;stored=texts[t];gets=0;errno=0;
  wgn_subnet_rule *result=get(nulls&1?NULL:rules,cap,nulls&2?NULL:&count);
  printf("result:%d:%zu:%u:%d:",result==rules?1:result==NULL?0:-1,count,gets,errno);unsigned char *b=(void *)rules;for(size_t i=0;i<sizeof(rules);i++)if(b[i]!=0xa5)printf("%zu=%02x,",i,b[i]);puts("");cases++;
 }
 void (*set)(wgn_subnet_rule *,size_t)=dlsym(h,"wgn_subnet_list_set_to_nvram");assert(set);
 for(int previous=0;previous<=8;previous++)for(unsigned kind=0;kind<3;kind++)for(size_t count=0;count<4;count++)for(unsigned incoming=0;incoming<3;incoming++)for(int nulls=0;nulls<2;nulls++){
  char value[4000]="";unsigned retained=0;for(int r=0;r<previous;r++){char row[300];unsigned type=kind==2?(unsigned)r%2:kind;retained+=type!=1;snprintf(row,sizeof(row),"<10.0.%d.1>255.255.255.0>1>start>end>3600>domain>dns>wins>0>binding>%u",r,type);strcat(value,row);}
  if(retained+count>8)continue;
  stored=value;wgn_subnet_rule rules[3];memset(rules,0,sizeof(rules));for(unsigned r=0;r<3;r++){snprintf(rules[r].ipaddr,17,"192.168.%u.1",r);strcpy(rules[r].ipmask,"255.255.255.0");rules[r].type=incoming==2?r%2:incoming;}
  gets=sets=commits=0;errno=0;set(nulls?NULL:rules,count);printf("write:%u:%u:%u:%d\n",gets,sets,commits,errno);cases++;
 }
 wgn_subnet_rule *(*find)(wgn_subnet_rule *,size_t,char *)=dlsym(h,"wgn_subnet_list_find");assert(find);
 const char *masks[]={"255.255.255.0","255.255.255.254","255.255.255.255","0.0.0.0","255.0.255.0","junk","0xffffff00","255.255.0.0"};
 const char *queries[]={NULL,"","1","192","192.168.1.1","192.168.1.1/","192.168.1.1/24","192.168.1.1/0","192.168.1.1/32","192.168.2.1/16","no match"};
 for(unsigned m=0;m<8;m++)for(unsigned q=0;q<11;q++)for(size_t count=0;count<4;count++)for(unsigned nulls=0;nulls<2;nulls++){
  wgn_subnet_rule rules[3];memset(rules,0,sizeof(rules));for(unsigned r=0;r<3;r++){snprintf(rules[r].ipaddr,17,"192.168.%u.1",r+1);strcpy(rules[r].ipmask,masks[(m+r)%8]);}
  wgn_subnet_rule *found=find(nulls?NULL:rules,count,(char *)queries[q]);printf("find:%ld\n",found?(long)(found-rules):-1L);cases++;
 }
 fprintf(stderr,"%u subnet settings/lookup comparisons\n",cases);return 0;
}
