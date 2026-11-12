#include "vlan.h"
#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static char *names;static char *stored;static int repeater;static unsigned enabled;static int enabled_value;
static int present,types[4];
char *wl_nvname(const char *name,int unit,int sub){static char key[64];assert(!strcmp(name,"nband")&&!sub);snprintf(key,sizeof(key),"wl%d_nband",unit);printf("name:%d;",unit);return key;}
char *nvram_get(const char *key){printf("get:%s;",key);if(!strcmp(key,"wl_ifnames"))return names;if(!strcmp(key,"vlan_rulelist"))return stored;int unit=-1;assert(sscanf(key,"wl%d_",&unit)==1);return unit>=0&&unit<4&&(present&(1<<unit))?"yes":NULL;}
int nvram_get_int(const char *key){printf("int:%s;",key);if(!strcmp(key,"re_mode"))return repeater;if(strstr(key,"_bss_enabled")){int u=-1,s=-1;assert(sscanf(key,"wl%d.%d_bss_enabled",&u,&s)==2);assert(u>=0&&u<4&&s>0);return enabled&(1u<<u)?enabled_value:0;}int unit=-1;assert(sscanf(key,"wl%d_nband_type",&unit)==1);assert(unit>=0&&unit<4);return types[unit];}
int nvram_set(const char *k,const char *v){(void)k;(void)v;abort();}
int nvram_commit(void){abort();}
int main(int argc,char **argv){assert(argc==2);void *h=dlopen(argv[1],RTLD_LAZY|RTLD_LOCAL);if(!h){puts(dlerror());return 1;}
 int (*get)(void)=dlsym(h,"wgn_guest_is_enabled");assert(get);unsigned cases=0;
 names="a b c d";present=15;types[0]=0;types[1]=2;types[2]=3;types[3]=4;
 char *values[]={NULL,"","bad","<1>20>3>a>b>0002>0000>sub>1>0>1","<1>20>3>a>b>0002>0000>sub>1>0>0","<1>20>3>a>b>0002>0000>sub>1>0>1<1>21>3>a>b>0004>0000>sub>1>0>1<1>22>3>a>b>0000>0002>sub>1>0>1<1>23>3>a>b>0000>00000002>sub>1>0>1<1>24>3>a>b>0000>000000000002>sub>1>0>1"};
 for(unsigned v=0;v<6;v++)for(repeater=0;repeater<=2;repeater++)for(enabled=0;enabled<16;enabled++)for(enabled_value=-1;enabled_value<=2;enabled_value++){
 stored=values[v];printf("enabled:%d\n",get());cases++;
 }
 fprintf(stderr,"%u guest enabled comparisons\n",cases);
}
