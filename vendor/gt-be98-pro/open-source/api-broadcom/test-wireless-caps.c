#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
static const char *interface_name,*caps,*re_mode;static int driver_result;
char *nvram_get(const char *key){const char *s=!strcmp(key,"re_mode")?re_mode:interface_name;printf("get:%s:%s;",key,s);return (char*)s;}
char *nvram_safe_get(const char *key){return nvram_get(key);}
int nvram_match(const char *key,const char *value){return !strcmp(nvram_get(key),value);}
int wl_iovar_get(char *ifname,char *key,void *buf,int len){assert(!strcmp(key,"cap"));assert(len>=1536);printf("driver:%s:%s:%d:%d;",ifname,key,len,driver_result);if(!driver_result)snprintf(buf,len,"%s",caps);return driver_result;}
int main(int argc,char **argv){assert(argc==2);void *h=dlopen(argv[1],RTLD_LAZY|RTLD_LOCAL);if(!h){fprintf(stderr,"%s",dlerror());return 1;}int(*f[2])(int)={dlsym(h,"wl_cap_mbss"),dlsym(h,"wl_max_no_vifs")};assert(f[0]&&f[1]);
const char *interfaces[]={"","eth0","wl0","wl3.15","interface-longer-than-usual"};
const char *values[]={"","mbss4","mbss8","mbss16","mbss32","unknown","mbss16 mbss4","mbss4 mbss16","mbss8 mbss16 mbss4"," mbss16  mbss8 ","MBSS16","mbss16extra","mbss16\tmbss4","unknown mbss16 unknown"};
const char *modes[]={"","0","1","2","01"};unsigned count=0;
for(unsigned a=0;a<5;a++)for(unsigned b=0;b<14;b++)for(unsigned m=0;m<5;m++)for(driver_result=-2;driver_result<=2;driver_result++)for(int unit=-1;unit<=5;unit++)for(unsigned fn=0;fn<2;fn++){
interface_name=interfaces[a];caps=values[b];re_mode=modes[m];printf("case:%u:%u:%u:%d:%d:%u;",a,b,m,driver_result,unit,fn);printf("result:%d\n",f[fn](unit));count++;
}fprintf(stderr,"%u wireless capability comparisons\n",count);return 0;}
