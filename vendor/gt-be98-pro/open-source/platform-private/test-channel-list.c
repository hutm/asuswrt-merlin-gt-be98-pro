#include <assert.h>
#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
static int result,ioresult,radioband,count,calls;static uint32_t specs[4095];static const char*ifname;
void dbg(const char*fmt,...){va_list ap;va_start(ap,fmt);vprintf(fmt,ap);va_end(ap);}
unsigned char wf_chspec_primary20_chan(uint16_t spec){calls++;printf("primary:%04x;",spec);return(unsigned char)spec;}
char *nvram_get(const char*key){printf("nv:%s:%s;",key,ifname?ifname:"NULL");return(char*)ifname;}
int wl_ioctl(char*name,int cmd,void*buf,int len){assert(cmd==141&&len==4);printf("band:%s:%d:%d;",name,ioresult,radioband);memcpy(buf,&radioband,4);return ioresult;}
int wl_iovar_getbuf(char*name,const char*key,void*param,int plen,void*buf,int len){assert(!strcmp(key,"chanspecs")&&plen==2&&len==16384);uint16_t filter;memcpy(&filter,param,2);assert(filter==0);unsigned char*p=buf;for(int i=0;i<len;i++)assert(!p[i]);printf("query:%s:%d:%d;",name,result,count);memcpy(buf,&count,4);if(count>0&&count<=4095)memcpy(p+4,specs,(size_t)count*4);return result;}
int main(int argc,char**argv){assert(argc==2||argc==3);void*h=dlopen(argv[1],RTLD_LAZY|RTLD_LOCAL);if(!h){puts(dlerror());return 1;}void(*fn)(int,int,int,char*,int,int)=dlsym(h,"wl_list_5g_chans");assert(fn);unsigned char buffer[258];
if(argc==3){ifname="eth0";radioband=1;count=INT32_MAX;memset(buffer,0xa5,sizeof(buffer));fn(0,1,0,(char*)buffer+1,256,0);assert(!buffer[1]&&buffer[0]==0xa5&&buffer[257]==0xa5&&!calls);fn(0,1,0,NULL,10,0);fn(0,1,0,(char*)buffer,0,0);fn(0,1,0,(char*)buffer,-1,0);assert(buffer[0]==0xa5);puts("4 channel list boundary cases passed");return 0;}
const char*names[]={NULL,"","eth0","interface_name_too_long"};int sizes[]={-1,0,1,17},lengths[]={1,2,6,7,8,14,256};unsigned cases=0;
for(int unit=0;unit<2;unit++)for(unsigned n=0;n<4;n++)for(ioresult=-1;ioresult<=1;ioresult++)for(radioband=0;radioband<3;radioband++)for(result=-1;result<=1;result++)for(unsigned c=0;c<4;c++){
 ifname=names[n];count=sizes[c];for(int i=0;i<count;i++)specs[i]=0xffff0000u|(((unsigned)i%8)<<11)|((37u*(unsigned)i+unit*13)&255);calls=0;memset(buffer,0xa5,sizeof(buffer));printf("case:%u;",cases++);fn(unit,1,0,(char*)buffer+1,256,0);assert(buffer[0]==0xa5&&buffer[257]==0xa5);printf("out:%s:%d\n",buffer+1,calls);
}
ifname="eth0";ioresult=0;radioband=1;result=0;count=31;
for(unsigned seed=0;seed<256;seed+=7)for(int band=-1;band<=6;band++)for(int war=-1;war<=1;war++)for(int bw=-1;bw<=5;bw++)for(unsigned l=0;l<7;l++){
 for(int i=0;i<count;i++)specs[i]=0xffff0000u|(((unsigned)i%8)<<11)|((seed+37u*(unsigned)i)&255);
 calls=0;memset(buffer,0xa5,sizeof(buffer));printf("case:%u;",cases++);fn(0,band,war,(char*)buffer+1,lengths[l],bw);assert(buffer[0]==0xa5&&buffer[lengths[l]+1]==0xa5);printf("out:%s:%d\n",buffer+1,calls);
}
int units[]={INT32_MIN,INT32_MAX,-1};for(unsigned u=0;u<3;u++){memset(buffer,0xa5,sizeof(buffer));calls=0;printf("case:%u;",cases++);fn(units[u],1,0,(char*)buffer+1,256,0);assert(buffer[0]==0xa5&&buffer[257]==0xa5);printf("out:%s:%d\n",buffer+1,calls);}
printf("%u channel list scenarios\n",cases);return 0;}
