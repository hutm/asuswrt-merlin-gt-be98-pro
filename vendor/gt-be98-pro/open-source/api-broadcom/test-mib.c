#include <assert.h>
#include <dlfcn.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
static int sock_result,fail_mask,call;static uint64_t seed;
int socket(int domain,int type,int protocol){assert(domain==AF_INET&&type==SOCK_DGRAM&&protocol==0);printf("socket;");return sock_result;}
int close(int fd){assert(fd==sock_result);printf("close:%d;",fd);return -1;}
int robo_ioctl2(int fd,int write,unsigned int page,unsigned int reg,void *buffer,int len){assert(fd==sock_result&&write==0&&len==8);uint64_t initial;memcpy(&initial,buffer,8);assert(initial==0);uint64_t value=seed+(uint64_t)reg;memcpy(buffer,&value,8);int fail=(fail_mask>>call++)&1;printf("read:%u:%u:%016llx:%d;",page,reg,(unsigned long long)value,fail);return fail?-1:0;}
int main(int argc,char **argv){assert(argc==2);void *h=dlopen(argv[1],RTLD_LAZY|RTLD_LOCAL);if(!h){fprintf(stderr,"%s",dlerror());return 1;}uint64_t (*fn)(int,char*)=dlsym(h,"get_phy_mib");assert(fn);char *names[]={"tx_bytes","rx_bytes","tx_packets","rx_packets","rx_crc_errors","","unknown","TX_BYTES"};int ports[]={INT_MIN,-33,-32,-1,0,1,4,8,31,255,INT_MAX};uint64_t seeds[]={0,1,UINT32_MAX,UINT64_C(0x123456789abcdef0),UINT64_MAX};unsigned cases=0;
for(unsigned p=0;p<11;p++)for(unsigned n=0;n<8;n++)for(unsigned s=0;s<5;s++)for(sock_result=-1;sock_result<=1;sock_result++)for(fail_mask=0;fail_mask<8;fail_mask++){seed=seeds[s];call=0;printf("result:%016llx\n",(unsigned long long)fn(ports[p],names[n]));cases++;}
fprintf(stderr,"%u MIB comparisons\n",cases);
}
