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
 void (*shift)(wgn_vlan_rule *,size_t)=dlsym(h,"wgn_vlan_list_wl_subunit_shift");assert(shift);unsigned cases=0;
 types[0]=0;types[1]=2;types[2]=3;types[3]=4;
 char *values[]={NULL,""," ","a","a b c d","  a  b c d  ","a b c d e f"};
 for(unsigned v=0;v<7;v++)for(present=0;present<16;present++)for(unsigned count=0;count<=8;count++)for(unsigned seed=0;seed<3;seed++){
  names=values[v];wgn_vlan_rule rules[8];memset(rules,0,sizeof(rules));
  for(unsigned i=0;i<8;i++){unsigned b=i%4;strcpy(rules[i].wl2gset,"0000");strcpy(rules[i].wl5gset,"0000");char *p=b?rules[i].wl5gset:rules[i].wl2gset;if(b>1){memset(p,'0',4*(b-1));p+=4*(b-1);}strcpy(p,seed==0?"0002":seed==1?"0004":i<4?"0002":"0004");}
  shift(rules,count);dump(rules,8);cases++;
 }
 shift(NULL,1);cases++;fprintf(stderr,"%u VLAN shift comparisons\n",cases);
}
