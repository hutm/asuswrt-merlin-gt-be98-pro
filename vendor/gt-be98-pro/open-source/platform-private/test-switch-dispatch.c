#include <assert.h>
#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
static uint64_t response;static int result,calls;
static int provider(int kind,int unit,unsigned int address,void *value,int length){uint64_t initial;memcpy(&initial,value,8);printf("call:%d:%d:%u:%d:%016llx;",kind,unit,address,length,(unsigned long long)initial);calls++;if(!(kind&1)){assert(initial==0);if(length>0&&length<=8)memcpy(value,&response,length);}return result;}
int bcm_reg_read_X(int unit,unsigned int addr,void *value,int len){assert(unit==1);return provider(0,unit,addr,value,len);}
int bcm_reg_write_X(int unit,unsigned int addr,void *value,int len){assert(unit==1);return provider(1,unit,addr,value,len);}
int bcm_pseudo_mdio_read(unsigned int addr,void *value,int len){return provider(2,0,addr,value,len);}
int bcm_pseudo_mdio_write(unsigned int addr,void *value,int len){return provider(3,0,addr,value,len);}
int main(int argc,char**argv){assert(argc==2);void*h=dlopen(argv[1],RTLD_LAZY|RTLD_LOCAL);if(!h){fprintf(stderr,"%s",dlerror());return 1;}uint64_t(*f)(int,unsigned int,int,int,uint64_t)=dlsym(h,"hnd_ethswctl");assert(f);int actions[]={INT_MIN,-1,0,1,2,INT_MAX};unsigned int addresses[]={0,1,0x100,0xffff,UINT_MAX};int lengths[]={-1,0,1,2,4,6,8,9,INT_MAX};int writes[]={-1,0,1,INT_MAX};int results[]={INT_MIN,-1,0,1,INT_MAX};uint64_t data[]={0,1,UINT32_MAX,UINT64_C(0x123456789abcdef0),UINT64_MAX};unsigned count=0;
for(unsigned a=0;a<6;a++)for(unsigned b=0;b<5;b++)for(unsigned l=0;l<9;l++)for(unsigned w=0;w<4;w++)for(unsigned r=0;r<5;r++)for(unsigned d=0;d<5;d++){
response=data[(d+1)%5];result=results[r];calls=0;printf("case:%u;",count);printf("result:%016llx\n",(unsigned long long)f(actions[a],addresses[b],lengths[l],writes[w],data[d]));assert(calls==(actions[a]==0||actions[a]==1));count++;}
fprintf(stderr,"%u switch dispatch comparisons\n",count);return 0;}
