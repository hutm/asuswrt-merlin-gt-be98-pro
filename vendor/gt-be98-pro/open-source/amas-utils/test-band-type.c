#include <assert.h>
#include <dlfcn.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
static int count,present,bands[4],number,type,result;
int num_of_wl_if(void){printf("count:%d;",count);return count;}
char *wl_nvname(const char *s,int unit,int sub){static char k[32];assert(!strcmp(s,"nband")&&sub==0&&unit>=0&&unit<4);printf("name:%d;",unit);snprintf(k,sizeof(k),"wl%d_nband",unit);return k;}
char *nvram_get(const char *key){int unit=-1;assert(sscanf(key,"wl%d_nband",&unit)==1&&unit>=0&&unit<4);printf("get:%s;",key);return present&(1<<unit)?(char *)"":NULL;}
int nvram_get_int(const char *key){int unit=-1;assert(sscanf(key,"wl%d_nband",&unit)==1&&unit>=0&&unit<4);printf("int:%s;",key);return bands[unit];}
int nvram_set_int(const char *key,int value){printf("set:%s:%d;",key,value);return result;}
int get_bandnum_by_nband(int band){printf("number:%d;",band);return number;}
int get_type_by_channel(int unit,int band){printf("type:%d:%d;",unit,band);return type;}
int main(int argc,char **argv){assert(argc==2);void *h=dlopen(argv[1],RTLD_LAZY|RTLD_LOCAL);assert(h);void (*fn)(void)=dlsym(h,"check_wlx_nband_type");assert(fn);unsigned cases=0;
 int vals[]={-1,0,1,2,4},edge[]={INT_MIN,-1,0,1,2,INT_MAX};
 for(count=-1;count<9;count++)for(present=0;present<16;present++)for(unsigned p=0;p<625;p++){
  unsigned v=p;for(int i=0;i<4;i++){bands[i]=vals[v%5];v/=5;}
  number=edge[p%6];type=edge[(p/6)%6];result=edge[(p/36)%6];fn();puts("done");cases++;
 }
 count=4;present=15;for(unsigned band=0;band<5;band++)for(unsigned n=0;n<6;n++)for(unsigned t=0;t<6;t++)for(unsigned r=0;r<6;r++){
  for(int i=0;i<4;i++)bands[i]=vals[band];
  number=edge[n];type=edge[t];result=edge[r];fn();puts("edges");cases++;
 }
 fprintf(stderr,"%u band type comparisons\n",cases);
}
