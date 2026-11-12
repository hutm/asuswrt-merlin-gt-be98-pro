#include <assert.h>
#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <sys/socket.h>
#include <net/if.h>
#include <limits.h>
static int sock_result,lookup_result,io_result,ret_value,closes,operations;static uint32_t output;static void *global;
int socket(int d,int t,int p){assert(d==AF_INET&&t==SOCK_DGRAM&&!p);printf("socket:%d;",sock_result);return sock_result;}
int close(int fd){assert(fd==sock_result);closes++;return 0;}
int cprintf(const char*fmt,...){va_list ap;va_start(ap,fmt);int n=vprintf(fmt,ap);va_end(ap);return n;}
int fprintf(FILE*s,const char*fmt,...){assert(s==stderr);va_list ap;va_start(ap,fmt);int n=vprintf(fmt,ap);va_end(ap);return n;}
int fputs(const char*s,FILE*f){assert(f==stderr);return printf("%s",s);}
int ioctl(int fd,unsigned long req,...){assert(fd==sock_result);va_list ap;va_start(ap,req);struct ifreq *ifr=va_arg(ap,struct ifreq*);va_end(ap);assert(!strcmp(ifr->ifr_name,"bcmsw"));if(req==0x8933){printf("lookup:%d;",lookup_result);return lookup_result;}assert(req==0x89f9&&ifr->ifr_data==global);operations++;uint32_t words[58];memcpy(words,global,sizeof(words));printf("query:%u:%u:%u:%u:%u;",words[0],words[7],words[8],words[13],words[4]);words[2]=ret_value;words[4]=output;memcpy(global,words,sizeof(words));return io_result;}
int main(int argc,char**argv){assert(argc==2||argc==3);int candidate=argc==3;void*h=dlopen(argv[1],RTLD_LAZY|RTLD_LOCAL);if(!h){puts(dlerror());return 1;}int(*fn)(char*,int,unsigned,unsigned,int)=dlsym(h,"ethctl_phy_op");global=dlsym(h,"ethctl");assert(fn&&global);char*types[]={"ext","int","extsw","i2c","10gserdes","10gpcs","serdespower","ext32","","unknown","EXT"};int addresses[]={INT_MIN,-1,0,1,31,32};unsigned regs[]={0,1,2,3,UINT_MAX};int writes[]={-1,0,1};unsigned count=0;
for(unsigned t=0;t<11;t++)for(unsigned a=0;a<6;a++)for(unsigned r=0;r<5;r++)for(unsigned w=0;w<3;w++)for(sock_result=-1;sock_result<=1;sock_result++)for(lookup_result=-1;lookup_result<=0;lookup_result++)for(io_result=-1;io_result<=1;io_result++)for(ret_value=-1;ret_value<=1;ret_value++){
memset(global,0xa5,232);closes=operations=0;output=count*UINT32_C(1234567);printf("case:%u;",count);int result=fn(types[t],addresses[a],regs[r],output^0xdeadbeef,writes[w]);int leak=!candidate&&operations&&writes[w]&&io_result==0&&ret_value==0;assert(closes==(sock_result>=0&&!leak));printf("result:%d:",result);for(int i=0;i<232;i++)printf("%02x",((unsigned char*)global)[i]);puts("");count++;}
printf("%u PHY operation comparisons\n",count);return 0;}
