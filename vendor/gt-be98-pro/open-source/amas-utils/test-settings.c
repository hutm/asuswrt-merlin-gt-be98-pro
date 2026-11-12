#include <assert.h>
#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
static int value,interfaces;
int nvram_get_int(const char *key){assert(!strcmp(key,"led_ctrl_cap")||!strcmp(key,"amas_ethernet"));printf("get:%s;",key);return value;}
int nvram_set_int(const char *key,int v){assert(!strcmp(key,"led_ctrl_cap"));printf("set:%s:%d;",key,v);value=v;return -1;}
int nvram_unset(const char *key){printf("unset:%s;",key);return -1;}
int num_of_wl_if(void){printf("count;");return interfaces;}
static int signed_bits(uint32_t bits){int v;memcpy(&v,&bits,4);return v;}
int main(int argc,char **argv){assert(argc==2);void *h=dlopen(argv[1],RTLD_LAZY|RTLD_LOCAL);assert(h);
 void (*add)(int)=dlsym(h,"add_led_ctrl_capability"),(*del)(int)=dlsym(h,"del_led_ctrl_capability"),(*unset)(void)=dlsym(h,"unset_selected_channel_info");int (*dwb)(void)=dlsym(h,"dwb_band_exist"),(*priority)(void)=dlsym(h,"is_connection_priority_is_wifi_first");assert(add&&del&&unset&&dwb&&priority);unsigned cases=0;
 int values[]={INT_MIN,INT_MIN+1,-1,0,1,2,3,4,7,15,16,31,255,256,65535,INT_MAX};
 for(unsigned i=0;i<16;i++)for(unsigned j=0;j<16;j++){
  value=values[i];add(values[j]);printf("value:%d\n",value);value=values[i];del(values[j]);printf("value:%d\n",value);cases+=2;
 }
 uint32_t state=0x12345678;for(unsigned i=0;i<10000;i++){state=state*1664525u+1013904223u;int initial=signed_bits(state);state=state*1664525u+1013904223u;int bits=signed_bits(state);value=initial;add(bits);printf("value:%d\n",value);value=initial;del(bits);printf("value:%d\n",value);cases+=2;}
 for(value=-100;value<=600;value++){printf("priority:%d\n",priority());cases++;}
 for(unsigned i=0;i<16;i++){value=values[i];printf("priority:%d\n",priority());interfaces=values[i];printf("dwb:%d\n",dwb());cases+=2;}
 for(interfaces=-2;interfaces<=20;interfaces++){unset();putchar('\n');cases++;}
 fprintf(stderr,"%u mesh settings comparisons\n",cases);
}
