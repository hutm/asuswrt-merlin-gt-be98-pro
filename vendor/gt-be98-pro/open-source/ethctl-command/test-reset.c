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
static int map,failstep,calls,answer,phy,scenario;
unsigned sleep(unsigned seconds){printf("sleep:%u\n",seconds);return 0;}
int ioctl(int fd,unsigned long number,...){
 va_list ap;va_start(ap,number);struct ifreq *ifr=va_arg(ap,struct ifreq *);va_end(ap);uint16_t words[4];memcpy(words,&ifr->ifr_ifru,8);calls++;
 printf("ioctl:%d:%lx:%s",fd,number,ifr->ifr_name);
 if(number==0x8947)printf(":port=%d",(int16_t)words[2]);
 if(number==0x8948)printf(":phy=%u:reg=%u:flags=%u",words[0],words[1],words[3]);
 if(number==0x89fa){uint32_t *d=(void *)ifr->ifr_data;printf(":op=%u:flags=%u:type=%u",d[0],d[6],d[7]);if(d[6]==3)printf(":unit=%d:port=%d",(int)d[4],(int)d[5]);}
 if(number==0x8949)printf(":phy=%u:reg=%u:value=%u:flags=%u",words[0],words[1],words[2],words[3]);
 puts("");errno=EIO;if(calls==failstep)return -1;
 if(number==0x89f4){*(int *)ifr->ifr_data=map;return 0;}
 if(number==0x89fa){uint32_t *d=(void *)ifr->ifr_data;d[58]=scenario&1?0x1000:0x40;d[8]=scenario&2?0:0x2000;d[9]=scenario&4?1:0;return 0;}
 if(number==0x8947){words[0]=phy;words[1]=0;words[2]=0;words[3]=(unsigned)phy>>16;}
 else if(number==0x8948){
 unsigned reg=words[1];unsigned values[32]={0};
 values[0]=scenario<8?((scenario&1?0x100:0)|(scenario&2?0x2000:0)|(scenario&4?0x40:0)):0x1000;
 values[1]=scenario==0?0:((scenario&1)?4:1);values[4]=0x3e0;values[5]=0x4000|(1u<<(5+scenario%5));
 values[9]=scenario&2?0x300:0;values[10]=scenario&4?0xc00:0;
 values[24]=scenario>=16?16:0;values[25]=(scenario%8)<<8;
 if(scenario==100)values[4]=0;
 words[3]=values[reg&31];
 }else assert(number==0x8949);
 memcpy(&ifr->ifr_ifru,words,8);return 0;
}
int main(int argc,char **argv){assert(argc==3);void *h=dlopen(argv[1],RTLD_LAZY|RTLD_LOCAL);assert(h);callback fn;
 if(atoi(argv[2]))fn=dlsym(h,"candidate_reset");else{const struct command *(*lookup)(const char *)=dlsym(h,"command_lookup");fn=lookup("phy-reset")->fn;}assert(fn);
 char *cases[][7]={{NULL},{"port","0",NULL},{"port","1",NULL},{"port","-1",NULL},{"port","32",NULL},{"garbage",NULL},{"port",NULL}};
 int maps[]={-1,0,1,2,3,4,0x40000000},phys[]={0,31,32,0x100005,-1};unsigned count=0;
 for(unsigned c=0;c<sizeof(cases)/sizeof(*cases);c++)for(unsigned m=0;m<7;m++)for(unsigned p=0;p<5;p++)for(failstep=0;failstep<16;failstep++)for(scenario=0;scenario<32;scenario++){
  char *args[12]={"ethctl","eth0","phy-reset"};memcpy(args+3,cases[c],sizeof(cases[c]));struct ifreq ifr;memset(&ifr,0,sizeof(ifr));strcpy(ifr.ifr_name,"eth0");map=maps[m];phy=phys[p];calls=0;errno=0;answer=0;
  int result=fn(23,&ifr,4,args);printf("result:%d:%d:%d\n",result,errno,calls);count++;
 }
 fprintf(stderr,"%u reset callback comparisons\n",count);return 0;
}
