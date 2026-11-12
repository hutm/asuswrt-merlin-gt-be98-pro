#include <assert.h>
#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>
static int fd_value,io_value,port,opens,closes,queries;static uint32_t seed;static unsigned slot;static const char *value;
int open64(const char *path,int flags,...){assert(!strcmp(path,"/dev/rtkswitch")&&flags==O_RDONLY);opens++;printf("open:%d;",fd_value);return fd_value;}
int close(int fd){assert(fd==fd_value);closes++;printf("close:%d;",fd);return 0;}
void perror(const char*s){printf("error:%s;",s);}
int ioctl(int fd,unsigned long req,...){assert(fd==fd_value&&req==1);queries++;va_list ap;va_start(ap,req);uint32_t *w=va_arg(ap,uint32_t*);va_end(ap);assert(w[0]==(uint32_t)port-1);for(int i=1;i<101;i++)assert(!w[i]);for(unsigned i=0;i<101;i++)w[i]=slot==101?seed+i:(i==slot?seed:0);printf("query:%d:%u:%u:%d;",port,seed,slot,io_value);return io_value;}
int f_read_string(const char *path,char *buf,int size){assert(size==32);snprintf(buf,size,"%s",value);printf("read:%s:%s;",path,buf);return strlen(buf);}
int main(int argc,char**argv){assert(argc==2||argc==3);void*h=dlopen(argv[1],RTLD_LAZY|RTLD_LOCAL);if(!h){fprintf(stderr,"%s",dlerror());return 1;}uint64_t(*fn)(int,char*)=dlsym(h,"rtk_get_phy_mib");assert(fn);unsigned count=0;char *types[]={NULL,"","tx_bytes","rx_bytes","tx_packets","rx_packets","rx_crc_errors","unknown","TX_BYTES"};uint32_t seeds[]={0,1,0x7fffffff,0x80000000,0xfffffffe,UINT32_MAX};const char *values[]={"","0","-1","18446744073709551615","18446744073709551616","12345junk"};
if(argc==2){
 for(port=0;port<=5;port++)for(unsigned t=0;t<9;t++)for(unsigned s=0;s<6;s++)for(slot=0;slot<=101;slot++)for(fd_value=-1;fd_value<=1;fd_value++)for(io_value=0;io_value<=1;io_value++){
 seed=seeds[s];value=values[s];opens=closes=queries=0;printf("case:%u;",count);printf("result:%016llx\n",(unsigned long long)fn(port,types[t]));assert(opens==!!(port&&types[t]));assert(queries==(opens&&fd_value>=0));assert(closes==queries);count++;
 }
}else{
 int ports[]={INT_MIN,-1,1,2,3,4,5,6,INT_MAX};value="0";seed=UINT32_MAX;slot=101;
 for(unsigned p=0;p<9;p++)for(fd_value=-1;fd_value<=2;fd_value++)for(unsigned t=0;t<9;t++){
 port=ports[p];io_value=-1;opens=closes=queries=0;assert(fn(port,types[t])==0);assert(opens==!!(types[t]&&port>=1&&port<=5));assert(queries==(opens&&fd_value>=0));assert(closes==queries);count++;
 }
}fprintf(stderr,"%u RTK counter cases\n",count);return 0;}
