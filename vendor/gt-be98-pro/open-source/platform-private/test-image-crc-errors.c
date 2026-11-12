#define main basic_test_main
#include "test-image-crc.c"
#undef main
#include <errno.h>
#include <string.h>
static unsigned char contents[102421];static size_t position,total,cap;static long fail_after;static int closes;
static ssize_t read_cookie(void*cookie,char*buf,size_t n){(void)cookie;if(fail_after>=0&&position>=(size_t)fail_after){errno=EIO;return -1;}if(position==total)return 0;if(n>cap)n=cap;if(n>total-position)n=total-position;if(fail_after>=0&&n>(size_t)fail_after-position)n=(size_t)fail_after-position;memcpy(buf,contents+position,n);position+=n;return(ssize_t)n;}
static int close_cookie(void*cookie){(void)cookie;closes++;return 0;}
FILE*fopen(const char*path,const char*mode){assert(!strcmp(path,"fixture")&&!strcmp(mode,"r"));cookie_io_functions_t ops={.read=read_cookie,.close=close_cookie};return fopencookie(NULL,"r",ops);}
FILE*fopen64(const char*path,const char*mode){return fopen(path,mode);}
int main(int argc,char**argv){assert(argc==2);for(unsigned i=0;i<256;i++){uint32_t c=i;for(unsigned b=0;b<8;b++)c=(c>>1)^((c&1)?0xedb88320u:0);table[i]=c;}for(size_t i=0;i<sizeof(contents);i++)contents[i]=(unsigned char)(i*37);void*h=dlopen(argv[1],RTLD_LAZY|RTLD_LOCAL);assert(h);uint32_t(*fn)(char*)=dlsym(h,"img_crc_hnd");assert(fn);size_t sizes[]={0,19,20,21,102400,102421},caps[]={1,7,8192,102400};long failures[]={-1,0,1,19,102400};unsigned cases=0;
for(unsigned s=0;s<6;s++)for(unsigned c=0;c<4;c++)for(unsigned f=0;f<5;f++){total=sizes[s];cap=caps[c];fail_after=failures[f];position=0;closes=0;uint32_t actual=fn("fixture");int failed=fail_after>=0&&(size_t)fail_after<=total;uint32_t expected=failed||total<20?0:oracle(contents,total-20);assert(actual==expected&&closes==1);printf("case:%u:size:%zu:cap:%zu:failure:%ld:crc:%08x\n",cases++,total,cap,fail_after,actual);}
printf("%u CRC stream cases passed\n",cases);return 0;}
