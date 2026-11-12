/* SPDX-License-Identifier: GPL-2.0-only */
#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "model.h"
struct sw_entry {char *name,*id,*key;unsigned flags;};
static unsigned faults,waits,commands;static int wrong_product;
time_t time(time_t *out){if(out)*out=1700000000;return 1700000000;}
void srand(unsigned seed){assert(seed==1700000000);}
int rand(void){return 123;}
char *get_productid(void){return wrong_product?"unknown":"GT-BE98_Pro";}
int get_model(void){return MODEL_GTBE98_PRO;}
char *get_lan_hwaddr(void){return "zero";}
char *get_wan_hwaddr(void){return "zero";}
char *get_2g_hwaddr(void){return "zero";}
int ether_atoe(const char *in,unsigned char *out){assert(!strcmp(in,"zero"));memset(out,0,6);return 1;}
int f_exists(const char *path){(void)path;return 0;}
int nvram_get_int(const char *key){if(!strcmp(key,"fw_check"))return 0;assert(!strcmp(key,"wlready"));return waits>=2;}
unsigned sleep(unsigned n){assert(n==1);waits++;assert(waits<=2);return 0;}
char *nvram_get(const char *key){
 if(!strcmp(key,"btn_rst_gpio"))return faults&1?"99":"4109";
 if(!strcmp(key,"btn_wps_gpio"))return faults&2?"99":"4108";
 if(!strcmp(key,"btn_led_gpio"))return faults&4?"99":"";
 if(!strcmp(key,"btn_wltog_gpio"))return faults&8?"99":"255";
 if(strstr(key,"ifname"))return "radio0";
 fprintf(stderr,"unexpected setting %s\n",key);abort();
}
char *wl_nvname(const char *name,int unit,int sub){static char key[64];assert(sub==0);snprintf(key,sizeof(key),"wl%d_%s",unit,name);return key;}
int system(const char *command){assert(!strncmp(command,"wl -i ",6));commands++;return 0;}
int f_read_string(const char *path,char *out,int cap){assert(!strcmp(path,"/tmp/hw_auth_clm")&&cap==32);snprintf(out,cap,"%s",faults&16?"mismatch":"GT-BE98 PRO version");return strlen(out);}
size_t strlcpy(char *d,const char *s,size_t n){size_t len=strlen(s);if(n){size_t k=len<n-1?len:n-1;memcpy(d,s,k);d[k]=0;}return len;}
int main(int argc,char **argv){
 assert(argc==2);void *h=dlopen(argv[1],RTLD_LAZY);if(!h){fputs(dlerror(),stderr);return 1;}
 int (*validate)(char *,char *)=dlsym(h,"auth_validate");struct sw_entry *sw=dlsym(h,"s_sw_auth_tuple");assert(validate&&sw);unsigned cases=0;
 for(unsigned app=0;app<5;app++)for(wrong_product=0;wrong_product<2;wrong_product++)for(faults=0;faults<32;faults++)for(unsigned wrong_key=0;wrong_key<2;wrong_key++){
  waits=commands=0;int result=validate(sw[app].id,wrong_key?"incorrect":sw[app].key);
  assert(result==(!wrong_product&&!faults&&!wrong_key));
  printf("CASE %u %d %u %u RESULT %d WAIT %u COMMAND %u\n",app,wrong_product,faults,wrong_key,result,waits,commands);cases++;
 }
 fprintf(stderr,"%u composed authentication cases passed\n",cases);return 0;
}
