#include <assert.h>
#include <dlfcn.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
static const char *present,*fallback,*value;static int mode,port,exists,read_len;static uint32_t status;
pid_t getpid(void){return 42;}
int cprintf(const char *fmt,...){va_list ap;va_start(ap,fmt);printf("debug:");int n=vprintf(fmt,ap);va_end(ap);return n;}
void syslog(int pri,const char *fmt,...){va_list ap;va_start(ap,fmt);printf("log:%d:",pri);vprintf(fmt,ap);puts("");va_end(ap);}
char *nvram_get(const char *key){assert(!strcmp(key,"sw_mode"));printf("get:%s:%s;",key,present?present:"NULL");return (char*)present;}
char *nvram_default_get(const char *key){assert(!strcmp(key,"sw_mode"));printf("default:%s:%s;",key,fallback);return (char*)fallback;}
int nvram_get_int(const char *key){int v;if(!strcmp(key,"sw_mode"))v=mode;else {assert(!strcmp(key,"wans_lanport"));v=port;}printf("int:%s:%d;",key,v);return v;}
int d_exists(const char *path){int v=strstr(path,"lower_eth1")?(exists&1):((exists>>1)&1);printf("dir:%s:%d;",path,v);return v;}
int f_read_string(const char *path,char *buf,int size){assert(size==32);int n=read_len;if(n>(int)strlen(value))n=strlen(value);if(n>size-1)n=size-1;if(n>0){memcpy(buf,value,n);}buf[n>0?n:0]=0;printf("read:%s:%d:%d;",path,size,n);return n;}
int open64(const char *path,int flags,...){assert(!strcmp(path,"/dev/rtkswitch")&&flags==O_RDONLY);printf("open:0;");return 0;}
int close(int fd){assert(fd==0);printf("close:0;");return 0;}
void perror(const char *s){printf("error:%s;",s);}
int ioctl(int fd,unsigned long req,...){assert(fd==0&&req==0);va_list ap;va_start(ap,req);uint32_t *v=va_arg(ap,uint32_t*);va_end(ap);for(int i=0;i<12;i++)assert(v[i]==0);for(int i=0;i<4;i++)v[i]=(status>>i)&1;printf("query:%u;",status);return 0;}
int main(int argc,char**argv){assert(argc==2);void*h=dlopen(argv[1],RTLD_LAZY|RTLD_LOCAL);if(!h){fprintf(stderr,"%s",dlerror());return 1;}uint32_t(*fn)(char*)=dlsym(h,"hnd_get_phy_status");assert(fn);
char longname[128];memset(longname,'x',127);longname[127]=0;char *ifs[]={"","eth0","eth1","vlan2","vlan4094","br101","br101x","br10",longname};const char *ps[]={NULL,"","1"};const char *defs[]={"","0","1","2","01","1junk"};const char *vals[]={"","up","upstream","down","downstream","unknown","UP"};int modes[]={-1,0,1,2};int ports[]={0,1,2,3,4};uint32_t statuses[]={0,1,2,UINT32_MAX};int lengths[]={-1,0,1,2,3,4,31};unsigned count=0;
for(unsigned i=0;i<9;i++)for(unsigned p=0;p<3;p++)for(unsigned d=0;d<6;d++)for(unsigned m=0;m<4;m++)for(exists=0;exists<4;exists++)for(unsigned v=0;v<7;v++)for(unsigned l=0;l<7;l++){
present=ps[p];fallback=defs[d];mode=modes[m];value=vals[v];read_len=lengths[l];port=ports[count%5];status=statuses[count%4];printf("case:%u;",count);printf("result:%u\n",fn(ifs[i]));count++;}
fprintf(stderr,"%u composed PHY status comparisons\n",count);return 0;}
