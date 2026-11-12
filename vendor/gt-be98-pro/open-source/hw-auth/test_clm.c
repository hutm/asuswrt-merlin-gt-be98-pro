/* SPDX-License-Identifier: GPL-2.0-only */
#include <assert.h>
#include <dlfcn.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
static int read_result;static char command[128],key[64];static unsigned commands,reads,names,gets;
int system(const char *s){snprintf(command,sizeof(command),"%s",s);commands++;return -1;}
int f_exists(const char *s){assert(!strcmp(s,"/tmp/HW_AUTH_DEBUG"));return 0;}
int f_read_string(const char *path,char *out,int cap){assert(!strcmp(path,"/tmp/hw_auth_clm")&&cap==32);reads++;strcpy(out,"version 1.2\n");return read_result;}
char *wl_nvname(const char *name,int unit,int subunit){assert(!strcmp(name,"ifname")&&subunit==0);names++;snprintf(key,sizeof(key),"wl%d_ifname",unit);return key;}
char *nvram_get(const char *name){assert(!strcmp(name,key));gets++;static char iface[64];snprintf(iface,sizeof(iface),"r-%.50s",name);return iface;}
int main(int argc,char **argv){
 assert(argc==2);
 void *h=dlopen(argv[1],RTLD_LAZY);if(!h){fputs(dlerror(),stderr);return 1;}
 char *symbols[]={"hw_component_clm_2g","hw_component_clm_5g1","hw_component_clm_5g2"};
 for(unsigned band=0;band<3;band++){
  char *(*fn)(int)=dlsym(h,symbols[band]);assert(fn);
  for(int m=-2;m<=514;m++)for(int result=-1;result<=1;result++){
   command[0]=key[0]=0;commands=reads=names=gets=0;read_result=result;
   char *out=fn(m);assert(out);printf("CASE %u %d %d %u %u %u %u [%s] [%s] [",band,m,result,commands,reads,names,gets,command,key);
   for(unsigned i=0;out[i];i++){assert(i<32);printf("%02x",(unsigned char)out[i]);}puts("]");
  }
 }
 return 0;
}
