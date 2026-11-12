#include <assert.h>
#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
static const char *wan,*lan;static unsigned seed,reads,calls;
char*nvram_get(const char*key){assert(!strcmp(key,"wan_ifnames")||!strcmp(key,"lan_ifnames"));const char*v=!strcmp(key,"wan_ifnames")?wan:lan;reads++;printf("get:%s:%s;",key,v?v:"NULL");return(char*)v;}
int hnd_get_phy_status(char*name){uint32_t hash=seed;for(const unsigned char*p=(void*)name;*p;p++)hash=hash*33+*p;int result=(int32_t)(hash&0x8000000f);calls++;printf("phy:%s:%d;",name,result);return result;}
int main(int argc,char**argv){assert(argc==2);void*h=dlopen(argv[1],RTLD_LAZY|RTLD_LOCAL);if(!h){puts(dlerror());return 1;}int(*fn)(void)=dlsym(h,"get_port_status_hnd");assert(fn);char long_name[300];memset(long_name,'x',sizeof(long_name));long_name[250]=' ';strcpy(long_name+251,"eth1");const char*values[]={NULL,""," ","   ","eth0","eth0 eth1"," eth0  eth1 ","eth0\teth1","eth0\neth1","eth0 eth0 eth0",long_name,"vlan2 vlan4094 br101 eth2"," eth1 "};unsigned count=0;
for(unsigned a=0;a<13;a++)for(unsigned b=0;b<13;b++)for(seed=0;seed<128;seed++){
wan=values[a];lan=values[b];reads=calls=0;printf("case:%u;",count++);int rc=fn();assert(reads==4);printf("result:%d:calls:%u\n",rc,calls);}
printf("%u port status comparisons\n",count);return 0;}
