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
static int map,failstep,calls,answer,phy;
int ioctl(int fd,unsigned long number,...){
 va_list ap;va_start(ap,number);struct ifreq *ifr=va_arg(ap,struct ifreq *);va_end(ap);uint16_t words[4];memcpy(words,&ifr->ifr_ifru,8);calls++;
 printf("ioctl:%d:%lx:%s",fd,number,ifr->ifr_name);
 if(number==0x8947)printf(":port=%d",(int16_t)words[2]);
 if(number==0x8948)printf(":phy=%u:reg=%u:flags=%u",words[0],words[1],words[3]);
 if(number==0x8949)printf(":phy=%u:reg=%u:value=%u:flags=%u",words[0],words[1],words[2],words[3]);
 puts("");errno=EIO;if(calls==failstep)return -1;
 if(number==0x89f4){*(int *)ifr->ifr_data=map;return 0;}
 if(number==0x8947){words[0]=phy;words[1]=0;words[2]=0;words[3]=(unsigned)phy>>16;}
 else if(number==0x8948)words[3]=answer;else assert(number==0x8949);
 memcpy(&ifr->ifr_ifru,words,8);return 0;
}
int main(int argc,char **argv){assert(argc==3);void *h=dlopen(argv[1],RTLD_LAZY|RTLD_LOCAL);assert(h);callback fn;
 if(atoi(argv[2]))fn=dlsym(h,"candidate_reg");else{const struct command *(*lookup)(const char *)=dlsym(h,"command_lookup");fn=lookup("reg")->fn;}assert(fn);
 char *cases[][7]={{NULL},{"0",NULL},{"31",NULL},{"32",NULL},{"-1",NULL},{"garbage",NULL},{"0x10","0xffff",NULL},{"3","7","9",NULL},{"port","1","3",NULL},{"3","port","0",NULL},{"3","port","-1",NULL},{"3","port","32",NULL},{"3","port","256",NULL},{"port","1","port","2","2",NULL},{"4294967296",NULL},{"-4294967296",NULL},{"2147483648",NULL},{"3","4294967296",NULL},{"3","-4294967296",NULL},{"3","port","2147483648",NULL},{"3","port","-2147483649",NULL}};
 int maps[]={-1,0,1,2,3,4,0x40000000},phys[]={0,31,32,0x20005,-1};unsigned count=0;
 for(unsigned c=0;c<sizeof(cases)/sizeof(*cases);c++)for(unsigned m=0;m<7;m++)for(unsigned p=0;p<5;p++)for(failstep=0;failstep<5;failstep++)for(answer=0;answer<2;answer++){
  char *args[12]={"ethctl","eth0","reg"};memcpy(args+3,cases[c],sizeof(cases[c]));struct ifreq ifr;memset(&ifr,0,sizeof(ifr));strcpy(ifr.ifr_name,"eth0");map=maps[m];phy=phys[p];calls=0;errno=0;
  int result=fn(23,&ifr,4,args);printf("result:%d:%d:%d\n",result,errno,calls);count++;
 }
 const char *numbers[]={"", " ", "+", "-", "-+1", "--1", "- 1", "++1", "+-1", "0", "00", "08", "010", "0x", "0x1f", "0X20", "1tail", "  -1", "4294967295", "4294967296", "-4294967295", "-4294967296", "18446744073709551615", "18446744073709551616", "-18446744073709551616", "2147483647", "2147483648", "-2147483648", "-2147483649"};
 for(unsigned n=0;n<sizeof(numbers)/sizeof(*numbers);n++)for(int kind=0;kind<3;kind++)for(int stale=0;stale<2;stale++){
  char *args[9]={"ethctl","eth0","reg","3",NULL};
  if(kind==0)args[3]=(char *)numbers[n];
  if(kind==1)args[4]=(char *)numbers[n];
  if(kind==2){args[4]="port";args[5]=(char *)numbers[n];}
  struct ifreq ifr;memset(&ifr,0,sizeof(ifr));strcpy(ifr.ifr_name,"eth0");map=3;phy=31;answer=0xffff;calls=0;failstep=0;errno=stale?ERANGE:0;
  int result=fn(23,&ifr,4,args);printf("numeric:%d:%d:%d\n",result,errno,calls);count++;
 }
 fprintf(stderr,"%u MII callback comparisons\n",count);return 0;
}
