#include <assert.h>
#include <dlfcn.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
static int mode,wl_count,eth_count,mlo,eap,debug,variant,scenario;
int nvram_get_int(const char *key){printf("int:%s;",key);
 if(!strcmp(key,"amas_ethernet"))return mode;
 if(!strcmp(key,"mld_enable"))return mlo;
 if(!strcmp(key,"amas_eap_bhmode"))return eap;
 if(!strcmp(key,"amas_dbg"))return debug;
 if(!strcmp(key,"aimesh_alg"))return variant;
 if(!strcmp(key,"amas_eth_bhmode"))return 0x123;
 if(!strcmp(key,"amas_wifi_bhmode"))return 0x456;
 if(!strcmp(key,"amas_costmode"))return 7;
 if(!strcmp(key,"amas_rssiscoremode"))return 8;
 if(strstr(key,"nband_type"))return variant%7;
 if(strstr(key,"nband"))return variant%3==0?2:variant%3==1?1:4;
 return 0;
}
char *nvram_get(const char *key){printf("get:%s;",key);
 if(!strcmp(key,"amas_ethif_type"))return (char *)"1 2";
 if(!strcmp(key,"wl_mlo_config"))return (char *)"enabled";
 if(!strcmp(key,"eth_ifnames"))return eth_count==0?(char *)"":eth_count==1?(char *)"eth4":(char *)"eth4 eth5";
 if(!strcmp(key,"sta_ifnames")||!strcmp(key,"wl_ifnames"))return wl_count==0?(char *)"":wl_count==1?(char *)"eth0":wl_count==2?(char *)"eth0 eth1":wl_count==3?(char *)"eth0 eth1 eth2":(char *)"eth0 eth1 eth2 eth3";
 if(!strcmp(key,"sta_priority")){static char *priorities[]={"", "1 2 3 4", "2 0 0 1 5 1 0 1 6 2 0 1", "2 0 0 0 5 1 0 0 6 2 0 0", "6 0 0 1 6 1 0 1", "5 0 0 1 5 1 0 1", "2 0 0 2 5 1 0 -1", "bad"};return priorities[scenario%8];}
 return (char *)"";
}
int nvram_set_int(const char *key,int value){printf("set:%s:%d;",key,value);return 0;}
char *wl_nvname(const char *name,int unit,int sub){static char key[64];printf("name:%s:%d:%d;",name,unit,sub);snprintf(key,sizeof(key),"wl%d_%s",unit,name);return key;}
int cprintf(const char *fmt,...){va_list ap;va_start(ap,fmt);int r=vprintf(fmt,ap);va_end(ap);return r;}
void dbg(const char *fmt,...){va_list ap;va_start(ap,fmt);vprintf(fmt,ap);va_end(ap);}
int f_exists(const char *path){printf("exists:%s;",path);return 0;}
char *nvram_pf_get(const char *prefix,const char *suffix){printf("pf:%s:%s;",prefix,suffix);return (char *)"wl0";}
int is_mlo_if(char *name){printf("mlo-if:%s;",name);return variant&1;}
int is_mlo_map(char *name){printf("mlo-map:%s;",name);return variant&2;}
int nvram_set(const char *key,const char *value){printf("set-string:%s:%s;",key,value);return 0;}
int nvram_commit(void){printf("commit;");return 0;}
int main(int argc,char **argv){assert(argc==2);void *h=dlopen(argv[1],RTLD_LAZY|RTLD_LOCAL);if(!h){fprintf(stderr,"%s",dlerror());return 1;}void (*fn)(int*,int*,int*,int*,int*)=dlsym(h,"trans_to_bhmode");assert(fn);unsigned cases=0;
 int modes[]={-1,0,1,2,3,4,5,6,7,8,9,10,11,19,20,30,39,40,49,50,59,60,69,70,79,80,89,90,99,100,200,300,400,999,1000,10000,100000,200000};
 for(scenario=0;scenario<8;scenario++)for(unsigned mi=0;mi<sizeof(modes)/sizeof(*modes);mi++)for(wl_count=0;wl_count<=4;wl_count++)for(eth_count=0;eth_count<=2;eth_count++)for(mlo=0;mlo<2;mlo++)for(variant=0;variant<4;variant++){
  mode=modes[mi];eap=variant&1;debug=variant&1;int a=scenario&1?0x101:0,b=0x202,c=-1,d=-1,e=scenario&2?0x404:0;
  printf("CASE:%u:%d:%d:%d:%d:%d\n",cases,mode,wl_count,eth_count,mlo,variant);
  fn(&a,&b,&c,&d,&e);printf("OUT:%08x:%08x:%08x:%08x:%08x\n",(unsigned)a,(unsigned)b,(unsigned)c,(unsigned)d,(unsigned)e);cases++;
 }
 fprintf(stderr,"%u backhaul reference scenarios\n",cases);
}
