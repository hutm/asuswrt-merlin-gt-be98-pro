#define _GNU_SOURCE
#include <assert.h>
#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
static uint32_t table[256];
void dbg(const char*fmt,...){va_list ap;va_start(ap,fmt);vprintf(fmt,ap);va_end(ap);}
#ifdef EXTERNAL_CRC
extern uint32_t hndcrc32(unsigned char*,unsigned int,uint32_t);
#else
uint32_t hndcrc32(unsigned char*p,unsigned int size,uint32_t crc){assert(size<=102400);for(unsigned i=0;i<size;i++)crc=(crc>>8)^table[(crc^p[i])&255];return crc;}
#endif
static uint32_t oracle(unsigned char*p,size_t n){uint32_t crc=UINT32_MAX;for(size_t i=0;i<n;i++){crc^=p[i];for(unsigned b=0;b<8;b++)crc=(crc>>1)^((crc&1)?0xedb88320u:0);}return crc;}
int main(int argc,char**argv){assert(argc==2||argc==3);for(unsigned i=0;i<256;i++){uint32_t c=i;for(unsigned b=0;b<8;b++)c=(c>>1)^((c&1)?0xedb88320u:0);table[i]=c;}void*h=dlopen(argv[1],RTLD_LAZY|RTLD_LOCAL);if(!h){puts(dlerror());return 1;}uint32_t(*fn)(char*)=dlsym(h,"img_crc_hnd");assert(fn);size_t normal[]={20,21,31,255,102399,102420,102421,204820,307231},boundaries[]={0,1,19,102400,102401,102419,204800,204801,204819};unsigned char *data=malloc(400000);assert(data);uint32_t state=42;for(unsigned i=0;i<400000;i++){state=state*1664525u+1013904223u;data[i]=(unsigned char)(state>>24);}char path[]="/tmp/image-crc-fixture";unsigned cases=0;
for(unsigned j=0;j<9;j++){size_t n=argc==3?boundaries[j]:normal[j];FILE*f=fopen(path,"wb");assert(f);assert(fwrite(data,1,n,f)==n);assert(!fclose(f));uint32_t expected=n<20?0:oracle(data,n-20),actual=fn(path);assert(actual==expected);printf("case:%u:length:%zu:crc:%08x\n",cases++,n,actual);}
assert(!unlink(path));assert(fn(path)==0);if(argc==3)assert(fn(NULL)==0);free(data);printf("%u CRC file cases passed\n",cases);return 0;}
