#include <assert.h>
#include <dlfcn.h>
#include <errno.h>
#include <net/if.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
typedef int (*callback)(int,struct ifreq *,int,char **);
struct command{int nargs;const char *name;callback fn;const char *help;};
static int map,failstep,calls,answer,phy,capability;
int ioctl(int fd,unsigned long number,...){
 va_list ap;va_start(ap,number);struct ifreq *ifr=va_arg(ap,struct ifreq *);va_end(ap);uint16_t words[4];memcpy(words,&ifr->ifr_ifru,8);calls++;
 printf("ioctl:%d:%lx:%s",fd,number,ifr->ifr_name);
 if(number==0x8947)printf(":port=%d",(int16_t)words[2]);
 if(number==0x8948)printf(":phy=%u:reg=%u:flags=%u",words[0],words[1],words[3]);
 if(number==0x8949)printf(":phy=%u:reg=%u:value=%u:flags=%u",words[0],words[1],words[2],words[3]);
 puts("");errno=EIO;if(calls==failstep)return -1;
 if(number==0x89f4){*(int *)ifr->ifr_data=map;return 0;}
 if(number==0x8947){words[0]=phy;words[1]=0;words[2]=0;words[3]=(unsigned)phy>>16;}
 else if(number==0x8948)words[3]=words[1]==9?capability:answer;else assert(number==0x8949);
 memcpy(&ifr->ifr_ifru,words,8);return 0;
}
int main(int argc,char **argv){assert(argc==3);void *h=dlopen(argv[1],RTLD_LAZY|RTLD_LOCAL);assert(h);callback fn;
 if(atoi(argv[2]))fn=dlsym(h,"candidate_wirespeed");else{const struct command *(*lookup)(const char *)=dlsym(h,"command_lookup");fn=lookup("ethernet@wirespeed")->fn;}assert(fn);
 char *cases[][7]={{"show",NULL},{"enable",NULL},{"disable",NULL},{"garbage",NULL},{"enable","port","0",NULL},{"show","port","1",NULL},{"disable","port","-1",NULL},{"enable","port","32",NULL},{"show","garbage","3",NULL},{"show","port","2147483648",NULL}};
 int maps[]={-1,0,1,2,3,4,0x40000000},phys[]={0,31,32,0x100005,-1};unsigned count=0;
 for(unsigned c=0;c<sizeof(cases)/sizeof(*cases);c++)for(unsigned m=0;m<7;m++)for(unsigned p=0;p<5;p++)for(failstep=0;failstep<10;failstep++)for(int value=0;value<3;value++)for(int cap=0;cap<3;cap++){
  char *args[12]={"ethctl","eth0","ethernet@wirespeed"};memcpy(args+3,cases[c],sizeof(cases[c]));struct ifreq ifr;memset(&ifr,0,sizeof(ifr));strcpy(ifr.ifr_name,"eth0");map=maps[m];phy=phys[p];answer=value==0?0:(value==1?0x10:0xffff);capability=cap==0?0:(cap==1?0x100:0x300);calls=0;errno=0;
  int result=fn(23,&ifr,4,args);printf("result:%d:%d:%d\n",result,errno,calls);count++;
 }
 fprintf(stderr,"%u wirespeed callback comparisons\n",count);return 0;
}
