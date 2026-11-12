#include <assert.h>
#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
static const char *mode,*name;static int result,count,pattern,delete_result,calls,fault;
static unsigned char target[3]={0,0x11,0xff};
int cprintf(const char *fmt,...){va_list ap;va_start(ap,fmt);int n=vprintf(fmt,ap);va_end(ap);return n;}
char *nvram_get(const char *key){const char*v=strstr(key,"mode")?mode:name;printf("nv:%s:%s;",key,v?v:"NULL");return(char*)v;}
int wl_del_ie(int unit,int sub,uint32_t flag,int len,unsigned char*oui,unsigned char*data){uint32_t h=2166136261u;for(int i=0;i<len-3;i++)h=(h^data[i])*16777619u;printf("delete:%d:%d:%x:%d:%02x%02x%02x:%08x:%d;",unit,sub,flag,len,oui[0],oui[1],oui[2],h,delete_result);calls++;return delete_result;}
int wl_ioctl(char*ifname,int cmd,void*buf,int len){assert(cmd==262&&len==16384);unsigned char*p=buf;assert(!memcmp(p,"vndr_ie",8));uint32_t flags;memcpy(&flags,p+8,4);assert(flags==UINT32_MAX&&p[12]==221);for(int i=13;i<len;i++)assert(!p[i]);printf("query:%s:%d;",ifname,result);if(result)return result;memset(p,0,len);memcpy(p,&count,4);
if(fault){
 int32_t badcount=INT32_MAX;memcpy(p,&badcount,4);
 if(fault<=3){p[15]=(unsigned char)(fault-1);return 0;}
 /* Fill valid, nonmatching records until a final record lacks room. */
 size_t off=4;while(off+15<=16384){p[off+11]=255;p[off+12]=0x55;off+=267;}
 return 0;
}
size_t pos=4;for(int i=0;i<count;i++){unsigned n=pattern==3?255:3+(unsigned)i%9;flags=(uint32_t)i*17;memcpy(p+pos,&flags,4);memset(p+pos+4,0xaa,6);p[pos+10]=221;p[pos+11]=n;memcpy(p+pos+12,target,3);if(pattern==0||(pattern==1&&(i%2)))p[pos+12]^=1;for(unsigned j=3;j<n;j++)p[pos+12+j]=(unsigned char)(i+j);pos+=12+n;assert(pos<16384);}return 0;}
int main(int argc,char**argv){assert(argc==2||argc==3);void*h=dlopen(argv[1],RTLD_LAZY|RTLD_LOCAL);if(!h){puts(dlerror());return 1;}void(*fn)(int,int,unsigned char*)=dlsym(h,"wl_del_ie_with_oui");assert(fn);
if(argc==3){mode="ap";name="eth0";result=0;count=1;for(fault=1;fault<=4;fault++){calls=0;fn(0,0,target);assert(!calls);}fn(0,0,NULL);puts("5 invalid response/pointer cases passed");return 0;}
const char*modes[]={NULL,"","ap","sta"};const char*names[]={NULL,"","eth0","interface_name_too_long"};int subs[]={-1,0,1,15},counts[]={-1,0,1,2,7,31};unsigned cases=0;
for(int unit=0;unit<2;unit++)for(unsigned s=0;s<4;s++)for(unsigned m=0;m<4;m++)for(unsigned n=0;n<4;n++)for(result=-1;result<=1;result++)for(unsigned c=0;c<6;c++)for(pattern=0;pattern<4;pattern++)for(delete_result=-1;delete_result<=1;delete_result++){
mode=modes[m];name=names[n];count=counts[c];calls=0;printf("case:%u;",cases++);fn(unit,subs[s],target);printf("calls:%d\n",calls);}
printf("%u vendor IE removal comparisons\n",cases);return 0;}
