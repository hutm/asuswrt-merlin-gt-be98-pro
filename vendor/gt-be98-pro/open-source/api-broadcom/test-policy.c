#include <assert.h>
#include <dlfcn.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static const char *names;static int selection;static char number[32];
int cprintf(const char *fmt,...){(void)fmt;return 0;}
char *nvram_get(const char *key){assert(!strcmp(key,"smart_connect_selif"));snprintf(number,sizeof(number),"%d",selection);printf("get:%s=%s\n",key,number);return number;}
char *nvram_safe_get(const char *key){return nvram_get(key);}
int nvram_get_int(const char *key){return atoi(nvram_get(key));}
int nvram_set(const char *key,const char *val){assert(strlen(key)<64&&strlen(val)<64);printf("set:%s=%s\n",key,val);return 0;}
int nvram_unset(const char *key){assert(strlen(key)<64);printf("unset:%s\n",key);return 0;}
void get_fh_sc_ifnames(unsigned int *bitmap,char *buf,int size){assert(size==64);*bitmap=0;snprintf(buf,size,"%s",names);printf("interfaces:%s\n",buf);}
int main(int argc,char **argv){assert(argc==2||argc==3);void *h=dlopen(argv[1],RTLD_LAZY|RTLD_LOCAL);if(!h){fprintf(stderr,"%s\n",dlerror());return 1;}
void(*gen)(int)=dlsym(h,"gen_bcmbsd_def_policy");int(*idx)(int)=dlsym(h,"bcmbsd_tblidx"),(*unit)(int)=dlsym(h,"get_unitid");assert(gen&&idx&&unit);
for(int x=-100;x<256;x++)printf("map:%d:%d:%d\n",x,idx(x),unit(x));
printf("extreme:%d:%d:%d:%d\n",idx(INT_MIN),idx(INT_MAX),unit(INT_MIN),unit(INT_MAX));
const char *lists[]={"","wl0 wl1 wl2 wl3","wl0.1 wl1.2 wl2.3 wl3.4","wl3 wl2 wl1 wl0","wl0.15 wl1.15 wl2.15 wl3.15","wl10 wl1"};unsigned count=0;
for(unsigned l=0;l<sizeof(lists)/sizeof(*lists);l++)for(selection=-1;selection<33;selection++)for(int sel=-1;sel<33;sel++) {names=lists[l];printf("case:%u:%d:%d\n",l,selection,sel);gen(sel);count++;}
if(argc==3){names="wl0.123456789012 wl1.123456789012 wl3.123456789012";for(int sel=0;sel<32;sel++){gen(sel);count++;}}
fprintf(stderr,"%u policy generation cases\n",count);return 0;}
