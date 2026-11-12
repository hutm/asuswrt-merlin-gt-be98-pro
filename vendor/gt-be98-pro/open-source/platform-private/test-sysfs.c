#include <assert.h>
#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
static const char *value;static int read_count,calls;
int f_read_string(const char *path,char *buffer,int max)
{
 assert(max==32);calls++;
 int n=read_count;if(n>(int)strlen(value))n=strlen(value);if(n>max-1)n=max-1;
 if(n>0) { memcpy(buffer,value,n); }
 buffer[n>0?n:0]=0;
 printf("read:%s:%d:%d;",path,max,n);return n;
}
int main(int argc,char **argv)
{
 assert(argc==2);void *h=dlopen(argv[1],RTLD_LAZY|RTLD_LOCAL);if(!h){fprintf(stderr,"%s",dlerror());return 1;}
 uint32_t(*speed)(char*)=dlsym(h,"hnd_get_phy_speed"),(*duplex)(char*)=dlsym(h,"hnd_get_phy_duplex");
 uint64_t(*mib)(char*,char*)=dlsym(h,"hnd_get_phy_mib");int(*board)(void)=dlsym(h,"is_rtl8372_boardid");assert(speed&&duplex&&mib&&board);assert(board()==1);
 char longname[128];memset(longname,'a',127);longname[127]=0;
 char *ifs[]={"","eth0","eth1","eth5","bond0","wl0.15",longname};
 char *types[]={"","tx_bytes","rx_bytes","rx_packets","rx_crc_errors","unknown",longname};
 const char *values[]={"","0","1","10","100","1000","2500","10000","4294967295","4294967296","18446744073709551615","18446744073709551616","-1","-2","-4294967295","-4294967296","+1000"," 1000\n","1000junk","0x100","junk","full","full\n","fuller","half","Full"," full","9999999999999999999999999999999999999999"};
 unsigned count=0;
 for(unsigned a=0;a<7;a++)for(unsigned v=0;v<sizeof(values)/sizeof(*values);v++)for(read_count=-1;read_count<=32;read_count++){
 value=values[v];calls=0;printf("speed:%u:%u:%d;",a,v,read_count);printf("%u\n",speed(ifs[a]));assert(calls==1);count++;
 calls=0;printf("duplex:%u:%u:%d;",a,v,read_count);printf("%u\n",duplex(ifs[a]));assert(calls==1);count++;
 for(unsigned b=0;b<7;b++){calls=0;printf("mib:%u:%u:%u:%d;",a,b,v,read_count);printf("%llu\n",(unsigned long long)mib(ifs[a],types[b]));assert(calls==1);count++;}
 }
 for(int a=0;a<2;a++)for(int b=0;b<2;b++)if(!a||!b){calls=0;assert(mib(a?"eth0":NULL,b?"tx_bytes":NULL)==0&&calls==0);count++;}
 fprintf(stderr,"%u sysfs comparisons\n",count);return 0;
}
