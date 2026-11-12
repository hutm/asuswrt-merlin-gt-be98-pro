#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
void dbg(const char*fmt,...){va_list ap;va_start(ap,fmt);vprintf(fmt,ap);va_end(ap);}
int main(int argc,char**argv){assert(argc==2);void*h=dlopen(argv[1],RTLD_LAZY|RTLD_LOCAL);assert(h);int(*fn)(char*,unsigned)=dlsym(h,"check_trx");assert(fn);uint32_t lengths[]={0,9444,9445,9448,0x8fdc30,0x8fdc31,UINT32_MAX};unsigned cases=0;char*path="/tmp/trx-real-fixture";
for(unsigned l=0;l<7;l++)for(unsigned seed=0;seed<32;seed++){
 uint32_t len=lengths[l];long a=len>9444?9444:16,b=len>0x8fdc30?0x8fdc30:len>9444?(long)(((len-16)/4-2357+4)*4):16;
 unsigned char header[28]={0};memcpy(header+4,&len,4);FILE*f=fopen(path,"wb");assert(f);assert(fwrite(header,1,28,f)==28);unsigned char first=(unsigned char)(seed*13),second=(unsigned char)(seed*17);assert(!fseek(f,a,SEEK_SET)&&fputc(first,f)!=EOF);assert(!fseek(f,b,SEEK_SET)&&fputc(second,f)!=EOF);assert(!fclose(f));if(a==b)first=second;
 unsigned expected=(unsigned char)(second?first-1-second:first%3-3);assert(fn(path,expected)==1);assert(fn(path,expected^255)==0);printf("case:%u:length:%u:expected:%u\n",cases++,len,expected);
}
assert(!unlink(path));puts("224 real-file TRX cases passed");return 0;}
