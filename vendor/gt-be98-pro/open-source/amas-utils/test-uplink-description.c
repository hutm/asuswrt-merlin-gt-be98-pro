#include <assert.h>
#include <dlfcn.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
struct entry {char name[32];int value;};
int main(int argc,char **argv){assert(argc==2);void *h=dlopen(argv[1],RTLD_LAZY|RTLD_LOCAL);assert(h);int (*fn)(char*,char*,char*,int)=dlsym(h,"gen_uplinkport_describe");assert(fn);unsigned cases=0;
 assert(sizeof(struct entry)==36&&offsetof(struct entry,value)==32);
 char *tables[]={"port_define","phy_type","phy_eth_subtype","phy_wifi_subtype","phy_plc_subtype","phy_moca_subtype"};
 for(unsigned t=0;t<6;t++){struct entry *p=dlsym(h,tables[t]);assert(p);for(unsigned i=0;;i++){printf("table:%s:%u:%s:%d\n",tables[t],i,p[i].name,p[i].value);if(p[i].value==-1)break;assert(i<32);}}
 char *names[]={NULL,"","NONE","WAN","LAN","ETH","WIFI","PLC","MOCA","10","100","1000","1G","2.5G","5G","10G","10GSFP+","2.4G","6G","unknown","eth","WAN "," WIFI"};
 int indexes[]={INT_MIN,-65536,-1,0,1,255,256,65535,INT_MAX};
 for(unsigned p=0;p<23;p++)for(unsigned t=0;t<23;t++)for(unsigned s=0;s<23;s++)for(unsigned i=0;i<9;i++){
  if(!names[t]&&names[s])continue;
  printf("result:%d\n",fn(names[p],names[t],names[s],indexes[i]));cases++;
 }
 if(getenv("DEFENSIVE"))assert(fn("WAN",NULL,"1G",1)==65537);
 fprintf(stderr,"%u uplink description comparisons\n",cases);
}
