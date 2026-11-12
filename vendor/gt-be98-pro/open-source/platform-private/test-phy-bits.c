#include <assert.h>
#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <sys/socket.h>
#include <net/if.h>
#include <unistd.h>
static int fd_value,faults,closes,reads,ops,first,second;static uint32_t original,write_response;static void *global;
int socket(int d,int t,int p){assert(d==AF_INET&&t==SOCK_DGRAM&&!p);printf("socket:%d;",fd_value);return fd_value;}
int close(int fd){assert(fd==fd_value);closes++;printf("close:%d;",fd);return 0;}
int usleep(useconds_t n){printf("sleep:%u;",(unsigned)n);return -1;}
int nvram_get_int(const char *key){assert(!strcmp(key,"ext_phy_model"));int v=reads++?second:first;printf("setting:%d;",v);return v;}
int cprintf(const char*fmt,...){va_list ap;va_start(ap,fmt);int n=vprintf(fmt,ap);va_end(ap);return n;}
int fprintf(FILE*s,const char*fmt,...){assert(s==stderr);va_list ap;va_start(ap,fmt);int n=vprintf(fmt,ap);va_end(ap);return n;}
int fputs(const char*s,FILE*f){assert(f==stderr);return printf("%s",s);}
int ioctl(int fd,unsigned long req,...){assert(fd==fd_value);va_list ap;va_start(ap,req);struct ifreq*ifr=va_arg(ap,struct ifreq*);va_end(ap);assert(!strcmp(ifr->ifr_name,"bcmsw"));if(req==0x8933){printf("lookup:%d;",faults&1);return faults&1?-1:0;}assert(req==0x89f9&&ifr->ifr_data==global);uint32_t w[58];memcpy(w,global,sizeof(w));assert(w[0]==(ops?6u:5u)&&w[13]==1);printf("query:%u:%u:%u:%u;",w[0],w[7],w[8],w[4]);int rc=ops?(faults&8?-1:0):(faults&2?-1:0);w[2]=ops?(write_response&1):(faults&4?1:0);w[4]=ops?write_response:original;memcpy(global,w,sizeof(w));ops++;return rc;}
int main(int argc,char**argv){assert(argc==2);void*h=dlopen(argv[1],RTLD_LAZY|RTLD_LOCAL);if(!h){puts(dlerror());return 1;}int(*fn)(unsigned,unsigned,int,unsigned,unsigned,unsigned)=dlsym(h,"extphy_bit_op");global=dlsym(h,"ethctl");assert(fn&&global);unsigned ranges[]={0,1,7,14,15,16,UINT32_MAX};uint32_t values[]={0,1,0xffff,0x10000,0x7fffffff,0x80000000,0x12345678,UINT32_MAX};unsigned count=0;
for(unsigned a=0;a<7;a++)for(unsigned b=0;b<7;b++)for(unsigned v=0;v<8;v++)for(int wr=-1;wr<=1;wr++)for(faults=0;faults<16;faults++)for(fd_value=-1;fd_value<=2;fd_value++){
memset(global,0xa5,232);first=(int)(count%5)-1;second=(int)((count/5)%5)-1;original=values[(v+3)%8];write_response=values[(v+5)%8];closes=reads=ops=0;unsigned delay=values[(v+2)%8];printf("case:%u;",count);int result=fn(values[v],values[(v+1)%8],wr,ranges[a],ranges[b],delay);assert(closes==(fd_value>=0));printf("result:%d:",result);for(int i=0;i<56;i++)printf("%02x",((unsigned char*)global)[i]);puts("");for(int i=56;i<232;i++)assert(((unsigned char*)global)[i]==0xa5);count++;
}printf("%u PHY bit comparisons\n",count);return 0;}
