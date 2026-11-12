#include <assert.h>
#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
static int interfaces,present,bands[4];
int num_of_wl_if(void){printf("count;");return interfaces;}
char *wl_nvname(const char *name,int unit,int sub){assert(!strcmp(name,"nband")&&!sub);static char key[32];snprintf(key,sizeof(key),"wl%d_nband",unit);printf("name:%d;",unit);return key;}
char *nvram_get(const char *key){int unit=-1;assert(sscanf(key,"wl%d_nband",&unit)==1);assert(unit>=0&&unit<4);printf("get:%s;",key);return present&(1<<unit)?"value":NULL;}
int nvram_get_int(const char *key){int unit=-1;assert(sscanf(key,"wl%d_nband",&unit)==1);assert(unit>=0&&unit<4);printf("int:%s;",key);return bands[unit];}
int main(int argc,char **argv){assert(argc==2);void *h=dlopen(argv[1],RTLD_LAZY|RTLD_LOCAL);assert(h);float (*cost)(float,int)=dlsym(h,"cal_plc_cost");int (*count)(int)=dlsym(h,"get_bandnum_by_nband");assert(cost&&count);unsigned cases=0;
 uint32_t inputs[]={0,0x80000000,0x3f800000,0xbf800000,0xc0000000,0xc0800000,0xc1000000,1,0x80000001,0x7f7fffff,0xff7fffff,0x7f800000,0xff800000};
 int rates[]={INT_MIN,-1,0,1,199,200,201,399,400,401,899,900,901,INT_MAX};
 for(unsigned i=0;i<13;i++)for(unsigned r=0;r<14;r++){float p,out;memcpy(&p,&inputs[i],4);out=cost(p,rates[r]);uint32_t bits;memcpy(&bits,&out,4);printf("cost:%08x\n",bits);cases++;}
 uint32_t state=0x12345678;for(unsigned i=0;i<10000;i++){state=state*1664525u+1013904223u;uint32_t bits=state;if((bits&0x7f800000u)==0x7f800000u)bits^=0x00800000;float p,out;memcpy(&p,&bits,4);out=cost(p,(int)(i%1200)-100);memcpy(&bits,&out,4);printf("cost:%08x\n",bits);cases++;}
 for(interfaces=-1;interfaces<=8;interfaces++)for(present=0;present<16;present++)for(int pattern=0;pattern<9;pattern++){
  for(int i=0;i<4;i++)bands[i]=(pattern+i)%7-1;
  for(int band=-2;band<=7;band++){printf("bands:%d\n",count(band));cases++;}
 }
 fprintf(stderr,"%u mesh helper comparisons\n",cases);
}
