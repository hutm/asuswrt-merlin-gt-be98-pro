#include <assert.h>
#include <dlfcn.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
static int result;
int nvram_set_int(const char *key,int value){printf("set:%s:%d:%d;",key,value,result);return result;}
int main(int argc,char **argv){assert(argc==2);void *h=dlopen(argv[1],RTLD_LAZY|RTLD_LOCAL);assert(h);void (*init)(int)=dlsym(h,"amas_stop_acsd_config_init");assert(init);unsigned cases=0;
 int results[]={INT_MIN,-1,0,1,INT_MAX};
 for(int model=-1024;model<=1024;model++)for(unsigned r=0;r<5;r++){result=results[r];init(model);printf("model:%d\n",model);cases++;}
 uint32_t seed=9;for(unsigned i=0;i<10000;i++){seed=seed*1664525u+1013904223u;int model;memcpy(&model,&seed,4);result=results[i%5];init(model);printf("model:%d\n",model);cases++;}
 init(INT_MIN);puts("min");init(INT_MAX);puts("max");cases+=2;
 fprintf(stderr,"%u channel initialization comparisons\n",cases);
}
