#include <assert.h>
#include <dlfcn.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
static int ethernet,count,result,commit_result;
int nvram_get_int(const char *key){assert(!strcmp(key,"amas_ethernet"));printf("get:%s;",key);return ethernet;}
int get_wl_count(void){printf("count:%d;",count);return count;}
int nvram_set(const char *key,const char *value){printf("set:%s:%s;",key,value);return result;}
int nvram_commit(void){printf("commit;");return commit_result;}
int main(int argc,char **argv){assert(argc==2);void *h=dlopen(argv[1],RTLD_LAZY|RTLD_LOCAL);assert(h);void (*fn)(void)=dlsym(h,"trans_to_amas_bhmode");assert(fn);unsigned cases=0;
 int edges[]={INT_MIN,-1,0,1,2,3,4,INT_MAX};
 for(unsigned e=0;e<8;e++)for(unsigned c=0;c<8;c++)for(unsigned r=0;r<8;r++)for(unsigned k=0;k<8;k++){
  ethernet=edges[e];count=edges[c];result=edges[r];commit_result=edges[k];fn();puts("done");cases++;
 }
 for(ethernet=-100;ethernet<=1100;ethernet++)for(count=-2;count<17;count++){result=0;commit_result=0;fn();puts("done");cases++;}
 fprintf(stderr,"%u backhaul setting comparisons\n",cases);
}
