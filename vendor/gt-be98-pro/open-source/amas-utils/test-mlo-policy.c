#define _GNU_SOURCE
#include <assert.h>
#include <dlfcn.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
static int enabled,ethernet,exists,match,skip,present,open_failure,eth_reads,changed;
static char *config;static int token;
int nvram_get_int(const char *key){printf("int:%s;",key);if(!strcmp(key,"mld_enable"))return enabled;if(!strcmp(key,"amas_ethernet"))return eth_reads++?changed:ethernet;if(!strcmp(key,"cap_re_vendor_match"))return match;assert(!strcmp(key,"skip_check_cap_vendor"));return skip;}
char *nvram_get(const char *key){printf("get:%s;",key);if(!strcmp(key,"wl_mlo_config"))return config;assert(!strcmp(key,"cap_re_vendor_match"));return present?"":NULL;}
int nvram_set(const char *key,const char *value){assert(!strcmp(key,"wifi_first_detect")&&!strcmp(value,"1"));printf("set:%s:%s;",key,value);return -1;}
int f_exists(const char *path){assert(!strcmp(path,"/tmp/non_mlo_connection"));printf("exists;");return exists;}
FILE *fopen64(const char *path,const char *mode){assert(!strcmp(path,"/tmp/non_mlo_connection")&&!strcmp(mode,"w"));printf("open;");return open_failure?NULL:(FILE *)&token;}
int fclose(FILE *f){assert(f==(FILE *)&token);printf("close;");return -1;}
int unlink(const char *path){assert(!strcmp(path,"/tmp/non_mlo_connection"));printf("unlink;");return -1;}
int cprintf(const char *format,...){va_list ap;va_start(ap,format);int n=vprintf(format,ap);va_end(ap);return n;}
int main(int argc,char **argv){assert(argc==2);void *h=dlopen(argv[1],RTLD_LAZY|RTLD_LOCAL);assert(h);const char *names[]={"checkMloWiFi","checkMatchMloCondition","checkMloConnectionChange"};int (*fn[3])(void);for(unsigned i=0;i<3;i++){fn[i]=dlsym(h,names[i]);assert(fn[i]);}
 int modes[]={-1,0,3,99,100,101,399,400,401};char *configs[]={NULL,"","0","value"};unsigned cases=0;
 for(enabled=-1;enabled<=1;enabled++)for(exists=-1;exists<=1;exists++)for(unsigned c=0;c<4;c++)for(unsigned e=0;e<9;e++)for(open_failure=0;open_failure<2;open_failure++){
  config=configs[c];ethernet=modes[e];changed=modes[(e+1)%9];present=e%2;match=(int)(e%3)-1;skip=(int)(e%3)-1;
  for(unsigned f=0;f<3;f++){eth_reads=0;printf("result:%d\n",fn[f]());cases++;}
 }
 for(present=0;present<2;present++)for(match=-1;match<=1;match++)for(skip=-1;skip<=1;skip++)for(open_failure=0;open_failure<2;open_failure++){
 enabled=1;exists=0;config="value";eth_reads=0;printf("result:%d\n",fn[1]());cases++;
 }
 fprintf(stderr,"%u composed MLO policy comparisons\n",cases);
}
