#include <assert.h>
#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
static unsigned char(*decoder)(uint16_t);
static int result,count,calls;static uint32_t specs[4095];
void dbg(const char*fmt,...){va_list ap;va_start(ap,fmt);vprintf(fmt,ap);va_end(ap);}
unsigned char wf_chspec_primary20_chan(uint16_t spec){calls++;printf("primary:%04x;",spec);return decoder?decoder(spec):(unsigned char)spec;}
int wl_iovar_getbuf(char*name,const char*key,void*param,int plen,void*buf,int len){assert(!strcmp(key,"chanspecs")&&plen==2&&len==16384);uint16_t filter;memcpy(&filter,param,2);assert(filter==0);unsigned char*p=buf;for(int i=0;i<len;i++)assert(!p[i]);printf("query:%s:%d:%d;",name,result,count);memcpy(buf,&count,4);if(count>0&&count<=4095)memcpy(p+4,specs,(size_t)count*4);return result;}
int main(int argc,char**argv){assert(argc==2||argc==3);if(argc==3&&strcmp(argv[2],"invalid")){void*d=dlopen(argv[2],RTLD_LAZY|RTLD_LOCAL);assert(d);decoder=dlsym(d,"wf_chspec_primary20_chan");assert(decoder);}
void*h=dlopen(argv[1],RTLD_LAZY|RTLD_LOCAL);if(!h){puts(dlerror());return 1;}int(*non_dfs)(char*)=dlsym(h,"with_non_dfs_chspec");uint16_t(*low)(char*,uint16_t)=dlsym(h,"select_band1_chspec_with_same_bw"),(*high)(char*,uint16_t)=dlsym(h,"select_band4_chspec_with_same_bw");uint16_t(*select_band)(char*,int,int,uint16_t)=dlsym(h,"select_chspec_with_band_bw");assert(non_dfs&&low&&high&&select_band);
if(argc==3&&!strcmp(argv[2],"invalid")){result=0;count=4096;assert(!non_dfs("eth0")&&!low("eth0",0)&&!high("eth0",0)&&!select_band("eth0",1,0,0));count=INT32_MAX;assert(!non_dfs("eth0")&&!low("eth0",0)&&!high("eth0",0)&&!select_band("eth0",1,0,0));assert(!calls);puts("8 oversized count cases passed");return 0;}
unsigned cases=0;int sizes[]={-1,0,1,3,17};
for(result=-1;result<=1;result++)for(unsigned n=0;n<5;n++)for(unsigned seed=0;seed<256;seed++)for(unsigned bw=0;bw<8;bw++){
 count=sizes[n];for(int i=0;i<count;i++)specs[i]=0xa5a50000u|((seed%4)<<14)|(((bw+(unsigned)i)%8)<<11)|((seed+(unsigned)i*37)&255);
 uint16_t current=(uint16_t)((bw<<11)|seed);calls=0;printf("case:%u;",cases++);int a=non_dfs("eth0");unsigned b=low("eth0",current),c=high("eth0",current);printf("results:%d:%u:%u:%d\n",a,b,c,calls);
}
for(result=-1;result<=1;result++)for(unsigned seed=0;seed<256;seed+=3)for(int band=-1;band<=6;band++)for(int bw=-1;bw<=5;bw++){
 count=17;for(int i=0;i<count;i++)specs[i]=0xffff0000u|((seed%4)<<14)|(((unsigned)i%8)<<11)|((seed+(unsigned)i*37)&255);calls=0;printf("case:%u;",cases++);unsigned r=select_band("wl1",band,bw,(uint16_t)((seed%8)<<11));printf("result:%u:%d\n",r,calls);
}
printf("%u channel selection scenarios\n",cases);return 0;}
