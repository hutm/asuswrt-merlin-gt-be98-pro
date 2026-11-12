#include <assert.h>
#include <dlfcn.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
static const char *names;
static unsigned calls;
void get_fh_sc_ifnames(unsigned int *bitmap, char *buffer, int size) {
 assert(size==64); *bitmap=0; snprintf(buffer,size,"%s",names); calls++;
}
int main(int argc,char **argv) {
 assert(argc==2); void *h=dlopen(argv[1],RTLD_LAZY|RTLD_LOCAL);
 if(!h){fprintf(stderr,"%s\n",dlerror());return 1;}
 char *(*fn)(char*,size_t,int)=dlsym(h,"smart_connect_get_active_prefix_by_bandid"); assert(fn);
 const char *lists[]={"","wl0","wl0 wl1 wl2 wl3","wl0.1 wl1.2 wl2.3 wl3.4","wl10 wl1","wl1 wl10","eth0 wl3.15"," wl0  wl1 ","wl-1 wl2147483647","wl0.12345678901"};
 int bands[]={INT_MIN,-1,0,1,2,3,4,10,INT_MAX};
 unsigned count=0;
 for(unsigned l=0;l<sizeof(lists)/sizeof(*lists);l++)for(unsigned b=0;b<sizeof(bands)/sizeof(*bands);b++)for(int size=0;size<=32;size++)for(int null=0;null<2;null++) {
 unsigned char bytes[48]; memset(bytes,0xa5,sizeof(bytes)); names=lists[l];calls=0;
 char *target=null?NULL:(char*)bytes+8; char *result=fn(target,size,bands[b]);
 assert(result==((!target||size<=0)?NULL:target));
 assert(calls==((!target||size<=0)?0:1));
 for(int i=0;i<8;i++)assert(bytes[i]==0xa5);
 for(int i=8+(size>0&&!null?size:0);i<48;i++)assert(bytes[i]==0xa5);
 printf("%u:%u:%d:%d:%u:",l,b,size,null,calls);
 for(unsigned i=0;i<sizeof(bytes);i++) { printf("%02x",bytes[i]); }
 puts("");count++;
 }
 fprintf(stderr,"%u prefix comparisons\n",count);return 0;
}
