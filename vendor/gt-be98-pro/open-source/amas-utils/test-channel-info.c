#include <assert.h>
#include <dlfcn.h>
#include <limits.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
static int repeater,debug,unit,result;
int nvram_get_int(const char *key){printf("get:%s;",key);if(!strcmp(key,"re_mode"))return repeater;assert(!strcmp(key,"acsd_debug"));return debug;}
int nvram_set_int(const char *key,int value){printf("set:%s:%d;",key,value);return result;}
int wl_unit(char *name){printf("unit:%s;",name);return unit;}
uint8_t wf_chspec_primary20_chan(uint16_t spec){printf("primary:%u;",(unsigned)spec);return (uint8_t)(spec ^ (spec>>8));}
void syslog(int priority,const char *fmt,...){assert(priority==LOG_NOTICE);printf("log:%d;",priority);va_list ap;va_start(ap,fmt);vprintf(fmt,ap);va_end(ap);}
int main(int argc,char **argv){assert(argc==2);void *h=dlopen(argv[1],RTLD_LAZY|RTLD_LOCAL);assert(h);void (*fn)(char*,uint16_t)=dlsym(h,"acsd_amasinfo");assert(fn);unsigned cases=0;
 for(unsigned spec=0;spec<65536;spec++)for(repeater=0;repeater<2;repeater++) {unit=3;debug=(int)(spec%4);result=-1;fn("wl2.1",(uint16_t)spec);puts("done");cases++;}
 int edges[]={INT_MIN,-1,0,1,4,INT_MAX};char *names[]={"wl0","eth1","wl9.3","abc","a b","wl\t","\377xy",""};
 for(unsigned r=0;r<6;r++)for(unsigned u=0;u<6;u++)for(unsigned d=0;d<6;d++)for(unsigned n=0;n<8;n++){
  repeater=edges[r];unit=edges[u];debug=edges[d];result=edges[(r+u+d)%6];if(repeater&&strlen(names[n])<3)continue;
  fn(names[n],0x1901);puts("edge");cases++;
 }
 fprintf(stderr,"%u channel info comparisons\n",cases);
}
