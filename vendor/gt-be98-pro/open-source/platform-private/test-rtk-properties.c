#include <assert.h>
#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>
#include <unistd.h>
static int fd_value=0,io_value=0,opens,closes,queries;static unsigned pattern;static const char *value;
pid_t getpid(void){return 42;}
int cprintf(const char *fmt,...){va_list ap;va_start(ap,fmt);printf("debug:");int n=vprintf(fmt,ap);va_end(ap);return n;}
void syslog(int pri,const char *fmt,...){va_list ap;va_start(ap,fmt);printf("log:%d:",pri);vprintf(fmt,ap);puts("");va_end(ap);}
int open64(const char *path,int flags,...){assert(!strcmp(path,"/dev/rtkswitch")&&flags==O_RDONLY);opens++;printf("open:%d;",fd_value);return fd_value;}
int close(int fd){assert(fd==fd_value);closes++;printf("close:%d;",fd);return 0;}
void perror(const char *s){printf("error:%s;",s);}
int ioctl(int fd,unsigned long req,...){assert(fd==fd_value&&req==0);queries++;va_list ap;va_start(ap,req);uint32_t *v=va_arg(ap,uint32_t*);va_end(ap);for(int i=0;i<12;i++)assert(v[i]==0);for(int i=0;i<4;i++){v[i]=(pattern>>i)&1;v[i+4]=(pattern/16+i)%8;v[i+8]=(pattern/128+i)%4;}printf("ioctl:%u:%d;",pattern,io_value);return io_value;}
int f_read_string(const char *path,char *buf,int size){assert(size==32);snprintf(buf,size,"%s",value);printf("read:%s:%s;",path,buf);return strlen(buf);}
int main(int argc,char **argv){assert(argc==2||argc==3);void*h=dlopen(argv[1],RTLD_LAZY|RTLD_LOCAL);if(!h){fprintf(stderr,"%s",dlerror());return 1;}uint32_t(*f[3])(int)={dlsym(h,"rtk_get_phy_speed"),dlsym(h,"rtk_get_phy_duplex"),dlsym(h,"rtk_get_phy_status")};assert(f[0]&&f[1]&&f[2]);unsigned count=0;const char *values[]={"","0","1000","2500","-1","4294967295","4294967296","full","fuller","half","Full","up","down","unknown","upstream","downstream"};
if(argc==2){
 for(unsigned n=0;n<16;n++)for(int port=0;port<=4;port++)for(pattern=0;pattern<512;pattern++)for(unsigned fn=0;fn<3;fn++){value=values[n];fd_value=pattern%3;io_value=pattern%2;opens=closes=queries=0;printf("case:%u;",count);printf("result:%u\n",f[fn](port));assert(opens==(port!=0)&&queries==opens&&closes==opens);count++;}
}else{
 int ports[]={INT_MIN,-1,1,2,3,4,5,INT_MAX};value="0";
 for(unsigned p=0;p<8;p++)for(fd_value=-1;fd_value<=1;fd_value++)for(io_value=-1;io_value<=0;io_value++)for(unsigned fn=0;fn<3;fn++){
  int valid=ports[p]>=1&&ports[p]<=4;if(valid&&fd_value>=0&&io_value==0)continue;
  opens=closes=queries=0;pattern=511;uint32_t result=f[fn](ports[p]);assert(result==0);assert(opens==valid);assert(queries==(valid&&fd_value>=0));assert(closes==queries);count++;
 }
}
fprintf(stderr,"%u RTK property cases\n",count);return 0;}
