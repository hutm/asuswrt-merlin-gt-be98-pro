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
struct command {int nargs;const char *name;callback fn;const char *help;};
static int status,mutate,calls;
int ioctl(int fd,unsigned long number,...){
 va_list ap;va_start(ap,number);struct ifreq *ifr=va_arg(ap,struct ifreq *);va_end(ap);calls++;
 assert(number==0x89f9);unsigned char *p=(void *)ifr->ifr_data;
 printf("ioctl:%d:%lx:%s:",fd,number,ifr->ifr_name);for(int i=0;i<232;i++)printf("%02x",p[i]);putchar('\n');
 if(mutate)for(int i=4;i<232;i++)p[i]=(unsigned char)(i+mutate);
 errno=EIO;return status;
}
int main(int argc,char **argv){assert(argc==3);void *h=dlopen(argv[1],RTLD_LAZY|RTLD_LOCAL);if(!h){puts(dlerror());return 1;}int candidate=atoi(argv[2]);
 const struct command *(*lookup)(const char *)=dlsym(h,"command_lookup");
 const char *commands[]={"stats","phy-power","vport"};const char *symbols[]={"candidate_stats","candidate_power","candidate_vport"};
 const char *options[]={"","up","down","enable","disable","query","UP","garbage",NULL};int results[]={0,-1,1,-5,27};unsigned cases=0;
 for(mutate=0;mutate<3;mutate++)for(unsigned c=0;c<3;c++)for(unsigned o=0;o<9;o++)for(unsigned r=0;r<5;r++)for(int ac=0;ac<7;ac++){
  if(c==2&&!options[o])continue;
  callback fn=candidate?(callback)dlsym(h,symbols[c]):lookup(commands[c])->fn;assert(fn);
  char *args[]={"ethctl","eth0",(char *)commands[c],(char *)options[o],"port","1",NULL};
  struct ifreq ifr;memset(&ifr,0,sizeof(ifr));strcpy(ifr.ifr_name,"eth0");char marker[8]={0};ifr.ifr_data=marker;
  status=results[r];calls=0;errno=0;int value=fn(17,&ifr,ac,args);
  printf("result:%d:%d:%d:%d\n",value,errno,calls,ifr.ifr_data!=marker);cases++;
 }
 fprintf(stderr,"%u Ethernet callback comparisons\n",cases);return 0;
}
