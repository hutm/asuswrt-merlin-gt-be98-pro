#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
static int enabled,exists,skip,present,stage,changed,reads;
static char *config;
int nvram_get_int(const char *key){printf("int:%s;",key);if(!strcmp(key,"mld_enable"))return enabled;if(!strcmp(key,"mlo_skip_re_selfopt"))return skip;assert(!strncmp(key,"amas_wlc",8)&&strstr(key,"_optmz"));return reads++?changed:stage;}
char *nvram_get(const char *key){printf("get:%s;",key);if(!strcmp(key,"wl_mlo_config"))return config;assert(!strcmp(key,"mlo_skip_re_selfopt"));return present?"":NULL;}
int nvram_set_int(const char *key,int value){assert(!strncmp(key,"amas_wlc",8)&&strstr(key,"_optmz")&&value==0);printf("set:%s:%d;",key,value);return -1;}
int f_exists(const char *path){assert(!strcmp(path,"/tmp/non_mlo_connection"));printf("exists;");return exists;}
int main(int argc,char **argv){assert(argc==2);void *h=dlopen(argv[1],RTLD_LAZY|RTLD_LOCAL);assert(h);int (*mode)(void)=dlsym(h,"isMloConnectionMode"),(*self)(int)=dlsym(h,"is_self_optmz_stage");assert(mode&&self);unsigned cases=0;
 char *configs[]={NULL,"","0","1","a b"};int bands[]={INT_MIN,-1,0,1,3,4,INT_MAX};
 for(enabled=-1;enabled<=1;enabled++)for(exists=-1;exists<=1;exists++)for(unsigned c=0;c<5;c++){
 config=configs[c];printf("mode:%d\n",mode());cases++;
 for(present=0;present<=1;present++)for(skip=-1;skip<=1;skip++)for(stage=-1;stage<=3;stage++)for(changed=-1;changed<=3;changed++)for(unsigned b=0;b<7;b++){
 reads=0;printf("self:%d\n",self(bands[b]));cases++;
 }
 }
 fprintf(stderr,"%u MLO mode/stage comparisons\n",cases);
}
