#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
static int present,phase;
char *nvram_get(const char *key){assert(!strcmp(key,"MLOsupportPhase"));printf("get;");return present?"":NULL;}
int nvram_get_int(const char *key){assert(!strcmp(key,"MLOsupportPhase"));printf("int;");return phase;}
int main(int argc,char **argv){assert(argc==2);void *h=dlopen(argv[1],RTLD_LAZY|RTLD_LOCAL);assert(h);int (*supports)(int,int)=dlsym(h,"isMLOConnectionSupported");assert(supports);unsigned cases=0;
 for(present=0;present<2;present++)for(phase=-1;phase<=4;phase++)for(int ap=-1;ap<=100;ap++)for(int sta=-1;sta<=100;sta++){printf("%d\n",supports(ap,sta));cases++;}
 int extremes[]={INT_MIN,INT_MAX,0,3,98};present=1;
 for(unsigned i=0;i<5;i++)for(unsigned j=0;j<5;j++)for(unsigned k=0;k<5;k++){phase=extremes[k];printf("%d\n",supports(extremes[i],extremes[j]));cases++;}
 fprintf(stderr,"%u MLO support comparisons\n",cases);
}
