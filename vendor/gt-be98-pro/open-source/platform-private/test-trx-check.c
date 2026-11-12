#define _GNU_SOURCE
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <dlfcn.h>
static unsigned char token;static uint32_t length;static unsigned first,second,opens,closes,seeks,reads;static int fault;
static FILE*stream(void){return(FILE*)&token;}
void dbg(const char*fmt,...){va_list ap;va_start(ap,fmt);vprintf(fmt,ap);va_end(ap);}
FILE*fopen(const char*name,const char*mode){assert(!strcmp(name,"fixture")&&!strcmp(mode,"r"));opens++;printf("open;");return fault==1?NULL:stream();}
FILE*fopen64(const char*n,const char*m){return fopen(n,m);}
int fclose(FILE*fp){assert(fp==stream());closes++;printf("close;");return 0;}
int fseek(FILE*fp,long offset,int whence){assert(fp==stream()&&whence==SEEK_SET);seeks++;printf("seek:%ld;",offset);if((fault==3&&seeks==1)||(fault==4&&seeks==2))return -1;if(fault==7)return 1;return 0;}
size_t fread(void*buf,size_t size,size_t n,FILE*fp){assert(fp==stream()&&size==1);reads++;printf("read:%zu;",n);if(reads==1){assert(n==28);memset(buf,0,28);memcpy((char*)buf+4,&length,4);return fault==2?27:28;}assert(n==1);if((fault==5&&reads==2)||(fault==6&&reads==3))return 0;*(unsigned char*)buf=(unsigned char)(reads==2?first:second);return 1;}
int main(int argc,char**argv){assert(argc==2||argc==3);void*h=dlopen(argv[1],RTLD_LAZY|RTLD_LOCAL);if(!h){puts(dlerror());return 1;}int(*fn)(char*,unsigned)=dlsym(h,"check_trx");assert(fn);unsigned count=0;
if(argc==3){for(fault=1;fault<=7;fault++){opens=closes=reads=seeks=0;assert(fn("fixture",0)==0);assert(opens==1&&closes==(fault!=1));printf("fault:%d passed\n",fault);}assert(fn(NULL,0)==0);puts("8 TRX failure cases passed");return 0;}
uint32_t lengths[]={0,28,9443,9444,9445,9448,0x8fdc2f,0x8fdc30,0x8fdc31,UINT32_MAX};
for(unsigned l=0;l<10;l++)for(first=0;first<256;first++)for(second=0;second<256;second+=17)for(unsigned e=0;e<3;e++){
 length=lengths[l];unsigned expected=e==0?(unsigned char)(second?first-1-second:first%3-3):e==1?0:UINT32_MAX;opens=closes=reads=seeks=0;printf("case:%u;",count++);int rc=fn("fixture",expected);assert(opens==1&&closes==1&&reads==3&&seeks==2);printf("result:%d\n",rc);}
printf("%u TRX comparisons\n",count);return 0;}
