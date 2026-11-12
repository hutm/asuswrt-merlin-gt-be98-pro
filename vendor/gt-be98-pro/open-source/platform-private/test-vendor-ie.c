#include <assert.h>
#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#ifdef NATIVE_FIXTURE
#define malloc fixture_malloc
#define calloc fixture_calloc
#define free fixture_free
#endif
extern void *__libc_malloc(size_t);extern void *__libc_calloc(size_t,size_t);extern void __libc_free(void*);
static union {max_align_t align;unsigned char bytes[512];} block;
static int active,fail,allocs,frees,driver_result;static const char *mode,*name;
void *malloc(size_t n){if(!active)return __libc_malloc(n);assert(n>=23&&n<=275);allocs++;printf("malloc:%zu:%d;",n,fail);return fail?NULL:block.bytes;}
void *calloc(size_t n,size_t size){if(!active)return __libc_calloc(n,size);assert(!size||n<=SIZE_MAX/size);void*p=malloc(n*size);if(p)memset(p,0,n*size);return p;}
void free(void*p){if(p==block.bytes){frees++;return;}__libc_free(p);}
int cprintf(const char *fmt,...){va_list ap;va_start(ap,fmt);int n=vprintf(fmt,ap);va_end(ap);return n;}
char *nvram_get(const char *key){const char *v=strstr(key,"mode")?mode:name;printf("nv:%s:%s;",key,v?v:"NULL");return (char*)v;}
int wl_ioctl(char *ifname,int cmd,void *buf,int len){assert(cmd==263&&len==16384);uint32_t hash=2166136261u;unsigned char*p=buf;for(int i=0;i<len;i++)hash=(hash^p[i])*16777619u;printf("driver:%s:%08x:%d;",ifname,hash,driver_result);return driver_result;}
static void invalid_pointers(int(*fn)(int,int,uint32_t,int,unsigned char*,unsigned char*),unsigned char*oui,unsigned char*data)
{
 active=1;allocs=frees=0;
 assert(fn(0,0,1,3,NULL,data)==-1);
 assert(fn(0,0,1,4,oui,NULL)==-1);
 assert(!allocs&&!frees);active=0;
}
int main(int argc,char**argv){assert(argc==2||argc==3);int candidate=argc==3;int invalid=candidate&&!strcmp(argv[2],"invalid");void*h=dlopen(argv[1],RTLD_LAZY|RTLD_LOCAL);if(!h){puts(dlerror());return 1;}int(*f[2])(int,int,uint32_t,int,unsigned char*,unsigned char*)={dlsym(h,"wl_add_ie"),dlsym(h,"wl_del_ie")};assert(f[0]&&f[1]);unsigned char oui[3]={0,0x11,0xff},data[252];for(int i=0;i<252;i++)data[i]=(unsigned char)i;if(invalid){puts("invalid pointer checks");invalid_pointers(f[0],oui,data);invalid_pointers(f[1],oui,data);puts("4 invalid pointer cases passed");return 0;}int subs[]={-1,0,1,15};const char*modes[]={NULL,"","ap","sta"};const char*names[]={NULL,"","eth0","interface_name_too_long"};uint32_t flags[]={0,1,127,128,UINT32_MAX};int lengths[]={-1,0,2,3,4,255,256};unsigned count=0;
for(int fn=0;fn<2;fn++)for(int unit=0;unit<2;unit++)for(unsigned s=0;s<4;s++)for(unsigned m=0;m<4;m++)for(unsigned n=0;n<4;n++)for(unsigned p=0;p<5;p++)for(unsigned l=0;l<7;l++)for(driver_result=-1;driver_result<=1;driver_result++)for(fail=0;fail<2;fail++){
 mode=modes[m];name=names[n];allocs=frees=0;printf("case:%u;",count);active=1;int rc=f[fn](unit,subs[s],flags[p],lengths[l],oui,data);active=0;int allocated=allocs&&!fail;int skipped=subs[s]>0&&(!mode||strcmp(mode,"ap"));assert(frees==(allocated&&(candidate||!skipped)));printf("result:%d\n",rc);count++;}
printf("%u vendor IE comparisons\n",count);return 0;}
