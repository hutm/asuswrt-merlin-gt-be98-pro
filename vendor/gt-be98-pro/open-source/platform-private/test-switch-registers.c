#include <assert.h>
#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <sys/socket.h>
#include <net/if.h>
#include <limits.h>
static int fd_value,fail_mask,io_result,lookups,closes,kind,length,unit;static unsigned address;static unsigned char input[8];
int socket(int domain,int type,int protocol){assert(domain==AF_INET&&type==SOCK_DGRAM&&!protocol);printf("socket:%d;",fd_value);return fd_value;}
int close(int fd){assert(fd==fd_value);closes++;printf("close:%d;",fd);return 0;}
void perror(const char *s){printf("error:%s;",s);}
static uint32_t word(const unsigned char*p,int off){uint32_t n;memcpy(&n,p+off,4);return n;}
int ioctl(int fd,unsigned long cmd,...){assert(fd==fd_value);va_list ap;va_start(ap,cmd);struct ifreq *ifr=va_arg(ap,struct ifreq*);va_end(ap);
 if(cmd==0x8933){assert(!strcmp(ifr->ifr_name,lookups?"eth0":"bcmsw"));int rc=(fail_mask>>lookups++)&1?-1:0;printf("lookup:%s:%d;",ifr->ifr_name,rc);return rc;}
 assert(cmd==0x89fa);unsigned char *p=(void*)ifr->ifr_data;assert(word(p,0)==(kind<2?19u:20u));assert(word(p,28)==(kind&1?3u:2u));if(kind<2)assert(word(p,8)==(uint32_t)unit);assert(word(p,216)==address&&word(p,220)==(unsigned)length);
 printf("query:%s:%d:%u:%d:%d;",ifr->ifr_name,kind,address,length,io_result);
 if(kind&1){unsigned n=kind==3?8:(unsigned)length;assert(!memcmp(p+138,input,n));for(unsigned i=0;i<n;i++)printf("%02x",p[138+i]);}
 for(int i=0;i<8;i++)p[138+i]=(unsigned char)(0xf0+i);
 return io_result;}
int main(int argc,char**argv){assert(argc==2||argc==3);void*h=dlopen(argv[1],RTLD_LAZY|RTLD_LOCAL);if(!h){fprintf(stderr,"%s",dlerror());return 1;}int(*reg[2])(int,unsigned,void*,int)={dlsym(h,"bcm_reg_read_X"),dlsym(h,"bcm_reg_write_X")};int(*mdio[2])(unsigned,void*,int)={dlsym(h,"bcm_pseudo_mdio_read"),dlsym(h,"bcm_pseudo_mdio_write")};assert(reg[0]&&reg[1]&&mdio[0]&&mdio[1]);unsigned count=0;int units[]={INT_MIN,-1,0,1};unsigned addresses[]={0,1,0x12345678,UINT_MAX};
for(kind=0;kind<4;kind++)for(fd_value=-1;fd_value<=1;fd_value++)for(fail_mask=0;fail_mask<4;fail_mask++)for(io_result=-1;io_result<=1;io_result++)for(length=0;length<=8;length++)for(unsigned u=0;u<4;u++)for(unsigned a=0;a<4;a++){
 unit=units[u];address=addresses[a];unsigned char buffer[24];memset(buffer,0xa5,sizeof(buffer));for(int i=0;i<8;i++)buffer[8+i]=input[i]=(unsigned char)(count+i);lookups=closes=0;printf("case:%u;",count);
 int rc=kind<2?reg[kind](unit,address,buffer+8,length):mdio[kind-2](address,buffer+8,length);printf("result:%d:",rc);for(int i=0;i<24;i++)printf("%02x",buffer[i]);puts("");for(int i=0;i<8;i++)assert(buffer[i]==0xa5&&buffer[16+i]==0xa5);assert(closes==(fd_value>=0));count++;
}fprintf(stderr,"%u register access comparisons\n",count);return 0;}
