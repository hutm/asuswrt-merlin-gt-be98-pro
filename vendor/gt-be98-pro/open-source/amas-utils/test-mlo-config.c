#include <assert.h>
#include <dlfcn.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
static int count,bands[8],enabled,config,result;
int num_of_wl_if(void){printf("count:%d;",count);return count;}
int nvram_get_int(const char *key){printf("get:%s;",key);if(!strcmp(key,"mld_enable"))return enabled;if(!strcmp(key,"mlo_config"))return config;
 for(int i=0;i<8;i++){char k[32];snprintf(k,sizeof(k),"wl%d_nband",i);if(!strcmp(k,key))return bands[i];}assert(0);return 0;}
int nvram_set_int(const char *key,int v){assert(!strcmp(key,"mlo_config"));printf("set:%s:%d;",key,v);if(!result)config=v;return result;}
int nvram_unset(const char *key){assert(!strcmp(key,"mlo_config"));printf("unset:%s;",key);if(!result)config=0;return result;}
int main(int argc,char **argv){assert(argc==2);void *h=dlopen(argv[1],RTLD_LAZY|RTLD_LOCAL);assert(h);int (*gen)(void)=dlsym(h,"gen_mlo_band");void (*check)(void)=dlsym(h,"check_mlo_config");assert(gen&&check);unsigned cases=0;
 int vals[]={-1,0,1,2,4};int edges[]={INT_MIN,-1,0,1,3,13,19,25,35,INT_MAX};
 for(count=-1;count<=8;count++){unsigned patterns=1;for(int i=0;i<count;i++)patterns*=5;for(unsigned p=0;p<patterns;p++){unsigned v=p;for(int i=0;i<count;i++){bands[i]=vals[v%5];v/=5;}printf("gen:%d\n",gen());cases++;}}
 for(count=-1;count<=4;count++){unsigned patterns=1;for(int i=0;i<count;i++)patterns*=5;for(unsigned p=0;p<patterns;p++)for(unsigned e=0;e<10;e++)for(unsigned c=0;c<10;c++)for(result=-1;result<=0;result++){
  unsigned v=p;for(int i=0;i<count;i++){bands[i]=vals[v%5];v/=5;}enabled=edges[e];config=edges[c];check();printf("state:%d\n",config);check();printf("repeat:%d\n",config);cases+=2;
 }}
 fprintf(stderr,"%u MLO configuration comparisons\n",cases);
}
