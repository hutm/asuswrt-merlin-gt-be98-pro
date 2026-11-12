#include <assert.h>
#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <fcntl.h>
static int fd_value,io_value,mode,extwan,opens,closes,queries,settings;static unsigned mask;static uint32_t eth_status;static const char *present,*fallback;
int open64(const char *path,int flags,...){assert(!strcmp(path,"/dev/rtkswitch")&&flags==O_RDONLY);opens++;printf("open:%d;",fd_value);return fd_value;}
int close(int fd){assert(fd==fd_value);closes++;printf("close:%d;",fd);return 0;}
void perror(const char *s){printf("error:%s;",s);}
int ioctl(int fd,unsigned long req,...){assert(fd==fd_value&&req==0);queries++;va_list ap;va_start(ap,req);uint32_t *v=va_arg(ap,uint32_t*);va_end(ap);for(int i=0;i<12;i++)assert(v[i]==0);for(int i=0;i<4;i++)v[i]=(mask&(1u<<i))?(0x80000000u+i):0;printf("query:%u:%d;",mask,io_value);return io_value;}
char *nvram_get(const char *key){assert(!strcmp(key,"sw_mode"));settings++;printf("get:%s;",present?present:"NULL");return (char*)present;}
char *nvram_default_get(const char *key){assert(!strcmp(key,"sw_mode"));printf("default:%s;",fallback);return (char*)fallback;}
int nvram_get_int(const char *key){int v;if(!strcmp(key,"sw_mode"))v=mode;else {assert(!strcmp(key,"wans_extwan"));v=extwan;}printf("int:%s:%d;",key,v);return v;}
uint32_t hnd_get_phy_status(char *ifname){assert(!strcmp(ifname,"eth0"));printf("phy:%s:%u;",ifname,eth_status);return eth_status;}
int main(int argc,char **argv){assert(argc==2||argc==3);void*h=dlopen(argv[1],RTLD_LAZY|RTLD_LOCAL);if(!h){fprintf(stderr,"%s",dlerror());return 1;}int(*f)(void)=dlsym(h,"rtk_lan_phy_status");assert(f);unsigned count=0;
const char *ps[]={NULL,"","1"};const char *defs[]={"","0","1","01"};uint32_t eth[]={0,1,2,UINT32_MAX};
if(argc==2){
 for(unsigned p=0;p<3;p++)for(unsigned d=0;d<4;d++)for(mode=-1;mode<=2;mode++)for(extwan=-1;extwan<=1;extwan++)for(mask=0;mask<16;mask++)for(unsigned e=0;e<4;e++)for(fd_value=0;fd_value<3;fd_value++)for(io_value=0;io_value<2;io_value++){
 present=ps[p];fallback=defs[d];eth_status=eth[e];opens=closes=queries=settings=0;printf("case:%u;",count);printf("result:%d\n",f());assert(opens==1&&closes==1&&queries==1&&settings==1);count++;
 }
}else{
 for(fd_value=-1;fd_value<3;fd_value++)for(io_value=-1;io_value<1;io_value++)for(mask=0;mask<16;mask++){
 if(fd_value>=0&&io_value>=0)continue;
 opens=closes=queries=settings=0;assert(f()==0);assert(opens==1&&queries==(fd_value>=0)&&closes==queries&&!settings);count++;
 }
}
fprintf(stderr,"%u aggregate LAN cases\n",count);return 0;}
