#include "vlan.h"
#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static char *names;
static int present,types[4];
char *wl_nvname(const char *name,int unit,int sub){static char key[64];assert(!strcmp(name,"nband")&&!sub);snprintf(key,sizeof(key),"wl%d_nband",unit);printf("name:%d;",unit);return key;}
char *nvram_get(const char *key){printf("get:%s;",key);if(!strcmp(key,"wl_ifnames"))return names;int unit=-1;assert(sscanf(key,"wl%d_",&unit)==1);return unit>=0&&unit<4&&(present&(1<<unit))?"yes":NULL;}
int nvram_get_int(const char *key){printf("int:%s;",key);int unit=-1;assert(sscanf(key,"wl%d_nband_type",&unit)==1);assert(unit>=0&&unit<4);return types[unit];}
static void dump(wgn_vlan_rule *rules,size_t count){const unsigned char *p=(void *)rules;for(size_t i=0;i<count*sizeof(*rules);i++)printf("%02x",p[i]);putchar('\n');}
int main(int argc,char **argv){assert(argc==2);void *h=dlopen(argv[1],RTLD_LAZY|RTLD_LOCAL);assert(h);
 void (*set)(int,int,wgn_vlan_rule *)=dlsym(h,"wgn_set_wl_unit");
 wgn_vlan_rule *(*find)(wgn_vlan_rule *,size_t,int,int)=dlsym(h,"wgn_vlan_list_find_unit");
 int (*conflict)(wgn_vlan_rule *,size_t,int,int)=dlsym(h,"wgn_vlan_list_check_subunit_conflict");assert(set&&find&&conflict);unsigned cases=0;
 names="a b c d";present=15;
 for(int type=-1;type<=7;type++)for(int unit=-1;unit<=4;unit++)for(int sub=-1;sub<=17;sub++)for(int null=0;null<2;null++){
  for(int i=0;i<4;i++)types[i]=type;
  wgn_vlan_rule rule;memset(&rule,0xa5,sizeof(rule));set(unit,sub,null?NULL:&rule);dump(&rule,1);cases++;
 }
 types[0]=0;types[1]=2;types[2]=3;types[3]=4;
 const char *masks[]={"0000","0001","0002","0004","0008","0016","0032","0064","0128","0256","0512","1024","2048","4096","8192","9999"};
 for(unsigned seed=0;seed<48;seed++)for(unsigned count=0;count<=10;count++)for(int unit=-1;unit<=4;unit++)for(int sub=0;sub<=5;sub++){
  wgn_vlan_rule rules[10];memset(rules,0,sizeof(rules));
  for(unsigned i=0;i<10;i++){
   unsigned m=(seed+i)%16,b=(seed/16+i)%4;
   strcpy(rules[i].wl2gset,"0000");strcpy(rules[i].wl5gset,"0000000000000000");
   if(b==0)strcpy(rules[i].wl2gset,masks[m]);else{memset(rules[i].wl5gset,'0',4*(b-1));strcpy(rules[i].wl5gset+4*(b-1),masks[m]);}
  }
  wgn_vlan_rule *r=find(rules,count,unit,sub);printf("find:%td\n",r?r-rules:-1);cases++;
  printf("conflict:%d\n",conflict(rules,count,unit,sub));dump(rules,10);cases++;
 }
 assert(!find(NULL,1,0,1));assert(!conflict(NULL,1,0,1));cases+=2;
 fprintf(stderr,"%u VLAN unit comparisons\n",cases);
}
