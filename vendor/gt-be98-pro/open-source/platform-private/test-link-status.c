#include <assert.h>
#include <dlfcn.h>
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
static const char *value;static int nread;static uint32_t status;
pid_t getpid(void){return 42;}
int cprintf(const char *fmt,...){va_list ap;va_start(ap,fmt);printf("debug:");int n=vprintf(fmt,ap);va_end(ap);return n;}
void syslog(int priority,const char *fmt,...){va_list ap;va_start(ap,fmt);printf("log:%d:",priority);vprintf(fmt,ap);puts("");va_end(ap);}
uint32_t rtk_get_phy_status(int port){assert(port==1);printf("rtk:%d:%u;",port,status);return status;}
int f_read_string(const char *path,char *buffer,int size){assert(size==32);int n=nread;if(n>(int)strlen(value))n=strlen(value);if(n>size-1)n=size-1;if(n>0){memcpy(buffer,value,n);}buffer[n>0?n:0]=0;printf("read:%s:%d:%d;",path,size,n);return n;}
int main(int argc,char **argv){assert(argc==2);void*h=dlopen(argv[1],RTLD_LAZY|RTLD_LOCAL);if(!h){fprintf(stderr,"%s",dlerror());return 1;}int(*fn)(char*)=dlsym(h,"ethctl_get_link_status");assert(fn);
char longname[128];memset(longname,'x',127);longname[127]=0;char *names[]={"","eth0","eth1","bond0","vlan4094","vlan4094.1",longname};const char *values[]={"","up","up\n","upper","down","down\n","downstream","unknown","dormant","lowerlayerdown","notpresent","UP"," down"};uint32_t statuses[]={0,1,2,UINT32_MAX};unsigned count=0;
for(unsigned a=0;a<7;a++)for(unsigned b=0;b<13;b++)for(nread=-1;nread<=32;nread++)for(unsigned s=0;s<4;s++){value=values[b];status=statuses[s];printf("case:%u:%u:%d:%u;",a,b,nread,s);printf("result:%d\n",fn(names[a]));count++;}
fprintf(stderr,"%u link-status comparisons\n",count);return 0;}
