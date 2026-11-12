#include "vlan.h"
#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static char *names;static char *stored;static int repeater;
static int present,types[4];
char *wl_nvname(const char *name,int unit,int sub){static char key[64];assert(!strcmp(name,"nband")&&!sub);snprintf(key,sizeof(key),"wl%d_nband",unit);printf("name:%d;",unit);return key;}
char *nvram_get(const char *key){printf("get:%s;",key);if(!strcmp(key,"wl_ifnames"))return names;if(!strcmp(key,"vlan_rulelist"))return stored;int unit=-1;assert(sscanf(key,"wl%d_",&unit)==1);return unit>=0&&unit<4&&(present&(1<<unit))?"yes":NULL;}
int nvram_get_int(const char *key){printf("int:%s;",key);if(!strcmp(key,"re_mode"))return repeater;int unit=-1;assert(sscanf(key,"wl%d_nband_type",&unit)==1);assert(unit>=0&&unit<4);return types[unit];}
int nvram_set(const char *k,const char *v){(void)k;(void)v;abort();}
int nvram_commit(void){abort();}
int main(int argc,char **argv){assert(argc==2);void *h=dlopen(argv[1],RTLD_LAZY|RTLD_LOCAL);if(!h){puts(dlerror());return 1;}
 char *(*get)(char *,char *,size_t)=dlsym(h,"wgn_guest_vlans");assert(get);unsigned cases=0;
 names="a b c d";present=15;types[0]=0;types[1]=2;types[2]=3;types[3]=4;
 char *values[]={NULL,"","bad","<1>20>3>a>b>0002>0000>sub>1>0>1","<1>20>3>a>b>0002>0000>sub>1>0>0","<1>20>3>a>b>0002>0000>sub>1>0>1<1>21>3>a>b>0004>0000>sub>1>0>1<1>22>3>a>b>0000>0002>sub>1>0>1<1>23>3>a>b>0000>00000002>sub>1>0>1<1>24>3>a>b>0000>000000000002>sub>1>0>1"};
 size_t caps[]={0,1,2,5,6,7,8,12,16,32,64,128};
 char *queries[]={NULL,""," ","wl0.1","wl0.1_suffix","wl0.1 wl1.1","wl1.1 wl0.1","bad wl0.1 bad","wl0.1 wl9.1","wl9.1 wl0.1","wl0.0","wl-1.1","wl0.-1","wl0.1junk","wl0.1 wl0.1"," wl0.1  wl2.1 wl3.1 ","wl0.2","wl0","wl0.1\twl1.1"};
 for(unsigned v=0;v<6;v++)for(unsigned q=0;q<19;q++)for(unsigned c=0;c<12;c++)for(int null=0;null<2;null++){
  stored=values[v];unsigned char buffer[128];memset(buffer,0xa5,sizeof(buffer));char *result=get(queries[q],null?NULL:(char *)buffer,caps[c]);
  printf("result:%d:",result==(char *)buffer?1:result==NULL?0:-1);for(unsigned i=0;i<128;i++)if(buffer[i]!=0xa5)printf("%u=%02x,",i,buffer[i]);putchar('\n');cases++;
 }
 fprintf(stderr,"%u guest VLAN name comparisons\n",cases);
}
