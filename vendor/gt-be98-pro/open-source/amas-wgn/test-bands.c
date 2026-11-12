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
int main(int argc,char **argv){assert(argc==2);void *h=dlopen(argv[1],RTLD_LAZY|RTLD_LOCAL);if(!h){puts(dlerror());return 1;}
 int (*unit)(int)=dlsym(h,"wgn_get_unit_by_band"),(*band)(int)=dlsym(h,"wgn_get_band_by_unit"),(*count)(int)=dlsym(h,"wgn_guest_ifcount");
 int (*decode)(int *,int *,char *,char *)=dlsym(h,"wgn_get_wl_band_unit");void (*rule_decode)(int *,int *,wgn_vlan_rule *)=dlsym(h,"wgn_get_wl_unit");assert(unit&&band&&count&&decode&&rule_decode);unsigned cases=0;
 char *name_cases[]={NULL,""," ","eth1","eth1 eth2","  eth1   eth2 eth3 eth4 ","a b c d e f","a\tb c","a\nb c"};
 for(unsigned n=0;n<9;n++)for(present=0;present<16;present++)for(int t=-1;t<=7;t++){
  names=name_cases[n];for(int j=0;j<4;j++)types[j]=(t+j+1)%9-1;
  for(int b=-1;b<=5;b++){printf("u=%d\n",unit(b));cases++;}
  for(int u=-1;u<=4;u++){printf("b=%d\n",band(u));cases++;}
 }
 names="a b c d";present=15;types[0]=0;types[1]=2;types[2]=3;types[3]=4;
 char *sets[]={NULL,"","0000","0001","0002","0003","0004","0008","0010","0016","9999","-001","+002"," 002","0x10","2abc","00000002","00010000","000000000004","0000000000000008","00000000000000000001"};
 for(unsigned a=0;a<21;a++)for(unsigned b=0;b<21;b++)for(int nulls=0;nulls<4;nulls++){
  int u=99,s=99;int result=decode(nulls&1?NULL:&u,nulls&2?NULL:&s,sets[a],sets[b]);printf("d=%d,%d,%d\n",result,u,s);cases++;
  wgn_vlan_rule r;memset(&r,0,sizeof(r));snprintf(r.wl2gset,sizeof(r.wl2gset),"%s",sets[a]?sets[a]:"");snprintf(r.wl5gset,sizeof(r.wl5gset),"%s",sets[b]?sets[b]:"");u=s=99;rule_decode(nulls&1?NULL:&u,nulls&2?NULL:&s,&r);printf("r=%d,%d\n",u,s);cases++;
 }
 for(int nulls=0;nulls<4;nulls++){int u=99,s=99;rule_decode(nulls&1?NULL:&u,nulls&2?NULL:&s,NULL);printf("null=%d,%d\n",u,s);cases++;}
 for(int b=-100;b<=100;b++){printf("count=%d\n",count(b));cases++;}
 fprintf(stderr,"%u band mapping comparisons\n",cases);
}
