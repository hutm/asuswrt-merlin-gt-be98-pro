#include <assert.h>
#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <net/if.h>
#include <limits.h>
static int expected_fd,result,calls;static unsigned phy;static int location;static uint16_t response;
char *strerror(int err){assert(err==EIO);return "mock IO error";}
int fprintf(FILE *stream,const char *fmt,...){assert(stream==stderr);va_list ap;va_start(ap,fmt);int n=vprintf(fmt,ap);va_end(ap);return n;}
int ioctl(int fd,unsigned long req,...){assert(fd==expected_fd&&req==0x8948);calls++;va_list ap;va_start(ap,req);struct ifreq *ifr=va_arg(ap,struct ifreq*);va_end(ap);uint16_t fields[4];memcpy(fields,&ifr->ifr_ifru,8);assert(fields[0]==(phy&31)&&fields[1]==(uint16_t)location&&fields[2]==0xa5a5&&fields[3]==(uint16_t)(phy>>16));printf("ioctl:%d:%s:%u:%u:%u:%u:%d;",fd,ifr->ifr_name,fields[0],fields[1],fields[2],fields[3],result);fields[3]=response;memcpy(&ifr->ifr_ifru,fields,8);errno=EIO;return result;}
int main(int argc,char**argv){assert(argc==2);void*h=dlopen(argv[1],RTLD_LAZY|RTLD_LOCAL);if(!h){puts(dlerror());return 1;}int(*fn)(int,struct ifreq*,unsigned,int)=dlsym(h,"mdio_read");assert(fn);int fds[]={INT_MIN,-1,0,1,INT_MAX};unsigned phys[]={0,1,31,32,0xffff,0x80000000,0x12345678,UINT_MAX};int regs[]={INT_MIN,-1,0,1,31,32,65535,65536,INT_MAX};int results[]={INT_MIN,-1,0,1,INT_MAX};uint16_t responses[]={0,1,0x8000,0xffff};unsigned count=0;
for(unsigned f=0;f<5;f++)for(unsigned p=0;p<8;p++)for(unsigned r=0;r<9;r++)for(unsigned e=0;e<5;e++)for(unsigned v=0;v<4;v++){
 union {struct ifreq align;unsigned char bytes[64];} storage;memset(storage.bytes,0xa5,64);strcpy(storage.align.ifr_name,"eth0");expected_fd=fds[f];phy=phys[p];location=regs[r];result=results[e];response=responses[v];calls=0;printf("case:%u;",count);int ret=fn(expected_fd,&storage.align,phy,location);assert(calls==1&&ret==(result<0?0:response));printf("result:%d:",ret);for(int i=0;i<64;i++)printf("%02x",storage.bytes[i]);puts("");for(int i=24;i<64;i++)assert(storage.bytes[i]==0xa5);count++;
}printf("%u MDIO comparisons\n",count);return 0;}
