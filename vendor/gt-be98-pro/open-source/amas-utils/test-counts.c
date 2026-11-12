#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
static char *value;
char *nvram_get(const char *key){assert(!strcmp(key,"sta_ifnames")||!strcmp(key,"eth_ifnames"));printf("get:%s;",key);return value;}
int main(int argc,char **argv){assert(argc==2);void *h=dlopen(argv[1],RTLD_LAZY|RTLD_LOCAL);assert(h);int (*wl)(void)=dlsym(h,"get_wl_count"),(*eth)(void)=dlsym(h,"get_eth_count");assert(wl&&eth);unsigned cases=0;
 value=NULL;printf("wl:%d\n",wl());printf("eth:%d\n",eth());cases+=2;
 char buf[129];for(unsigned length=0;length<129;length++)for(unsigned pattern=0;pattern<12;pattern++){
  for(unsigned i=0;i<length;i++)buf[i]=(i+pattern)%(pattern+1)==0?' ':"ab\t\n"[(i+pattern)%4];
  buf[length]=0;value=buf;
  printf("wl:%d\n",wl());printf("eth:%d\n",eth());cases+=2;
 }
 fprintf(stderr,"%u interface count comparisons\n",cases);
}
