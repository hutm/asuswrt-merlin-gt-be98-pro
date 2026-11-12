#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
static char *defaults[2],*stored[2];
static unsigned index_of(const char *key){if(!strcmp(key,"vlan_rulelist"))return 0;assert(!strcmp(key,"subnet_rulelist"));return 1;}
char *nvram_default_get(const char *key){printf("default:%s;",key);return defaults[index_of(key)];}
char *nvram_get(const char *key){printf("get:%s;",key);return stored[index_of(key)];}
int nvram_set(const char *key,const char *value){assert(!strcmp(value,defaults[index_of(key)]));printf("set:%s:%s;",key,value);return -1;}
int nvram_commit(void){printf("commit;");return -1;}
int main(int argc,char **argv){assert(argc==2);void *h=dlopen(argv[1],RTLD_LAZY|RTLD_LOCAL);assert(h);void (*check)(void)=dlsym(h,"wgn_check_settings");assert(check);unsigned cases=0;
 char *values[]={NULL,"","a","ab","abc","<1>20>0","<1>20>0<1>21>1","other"};
 for(unsigned a=0;a<8;a++)for(unsigned b=0;b<8;b++)for(unsigned c=0;c<8;c++)for(unsigned d=0;d<8;d++){
 defaults[0]=values[a];defaults[1]=values[b];stored[0]=values[c];stored[1]=values[d];check();putchar('\n');cases++;
 }
 fprintf(stderr,"%u default settings comparisons\n",cases);
}
