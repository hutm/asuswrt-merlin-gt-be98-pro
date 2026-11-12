/* SPDX-License-Identifier: GPL-2.0-only */
#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include "prefixes.h"
static unsigned mac_calls,mac_slot=3;static uint32_t mac_prefix;
struct sw_entry {char *name,*id,*key;unsigned flags;};
struct mode_entry {char *product;int mode;};
static char *product;static int unlock,fwcheck,ready_after;static unsigned hash_calls,ready_calls,sleeps;
char *get_lan_hwaddr(void){puts("lan");return "zero";}
char *get_wan_hwaddr(void){puts("wan");return "zero";}
char *get_2g_hwaddr(void){puts("wifi");return "zero";}
int ether_atoe(const char *s,unsigned char *out){assert(!strcmp(s,"zero"));memset(out,0,6);if(mac_calls++==mac_slot){out[0]=mac_prefix>>24;out[1]=mac_prefix>>16;out[2]=mac_prefix>>8;}puts("parse-mac");return 1;}
int f_exists(const char *path){printf("exists %s\n",path);return strstr(path,"/unlock")?unlock:0;}
int nvram_get_int(const char *key){printf("get-int %s\n",key);if(!strcmp(key,"fw_check"))return fwcheck;assert(!strcmp(key,"wlready"));return ready_calls++>=(unsigned)ready_after;}
unsigned sleep(unsigned n){assert(n==1);sleeps++;assert(sleeps<5);puts("sleep");return 0;}
char *get_productid(void){puts("product");return product;}
char *gen_rand_value(char *excluded){printf("random-excluding %s\n",excluded);return "random-value";}
char *DoHardwareCheck(char *key){printf("hardware %s\n",key);return "hardware-id";}
char *get_auth_code(char *in,char *out,int cap){assert(cap==65);printf("hash [%s]\n",in);snprintf(out,cap,"%s",hash_calls++?"second":"first");return out;}
int system(const char *s){(void)s;abort();}
int main(int argc,char **argv){
 assert(argc==2);
 void *h=dlopen(argv[1],RTLD_LAZY);if(!h){fputs(dlerror(),stderr);return 1;}
 char *(*fn)(char *,char *,time_t,char *,int)=dlsym(h,"hw_auth_check");struct sw_entry *sw=dlsym(h,"s_sw_auth_tuple");struct mode_entry *amas=dlsym(h,"s_amas_whitelist_tuple");char **tunnel=dlsym(h,"s_tunnel_whitelist_tuple");assert(fn&&sw&&amas&&tunnel);
 struct sw_entry oldsw[6];memcpy(oldsw,sw,sizeof(oldsw));struct mode_entry oldamas=amas[0];char *oldtunnel=tunnel[0];memset(sw,0,sizeof(oldsw));
 char *apps[]={"ordinary","AMASH","aaews","mastiff"};char *products[]={"GT-BE98_Pro","unknown","RT-AC68U","RT-AC68U_V4"};unsigned cases=0;
 for(unsigned app=0;app<4;app++)for(unsigned p=0;p<4;p++)for(int allowed=0;allowed<3;allowed++)for(int matched=0;matched<2;matched++)for(int valid=0;valid<2;valid++)for(int blocked=0;blocked<3;blocked++){
  sw[0]=(struct sw_entry){apps[app],"app-id","test-key",0};product=products[p];amas[0]=(struct mode_entry){allowed==0?NULL:allowed==1?product:"different",3};tunnel[0]=amas[0].product;
  unlock=blocked==1;fwcheck=blocked==2;ready_after=2;hash_calls=ready_calls=sleeps=0;char out[65];memset(out,0x5a,sizeof(out));
  printf("CASE %u %u %d %d %d %d\n",app,p,allowed,matched,valid,blocked);
  assert(fn(matched?"app-id":"unknown-id",valid?"first":"wrong",1700000000,out,sizeof(out))==out);printf("RESULT %s %u %u %u\n",out,hash_calls,ready_calls,sleeps);cases++;
 }
 for(unsigned prefix=0;prefixes[prefix];prefix++)for(unsigned slot=0;slot<2;slot++)for(unsigned neighbor=0;neighbor<2;neighbor++){
  sw[0]=(struct sw_entry){"ordinary","app-id","test-key",0};product="GT-BE98_Pro";unlock=fwcheck=0;ready_after=0;hash_calls=ready_calls=sleeps=mac_calls=0;mac_slot=slot?2:0;mac_prefix=(prefixes[prefix]<<3)^(neighbor?256u:0u);char out[65];
  printf("PREFIX %u %u %u\n",prefix,slot,neighbor);assert(fn("app-id","first",1700000000,out,sizeof(out))==out);printf("RESULT %s %u %u %u\n",out,hash_calls,ready_calls,sleeps);cases++;
 }
 memcpy(sw,oldsw,sizeof(oldsw));amas[0]=oldamas;tunnel[0]=oldtunnel;fprintf(stderr,"%u handshake reference cases\n",cases);return 0;
}
