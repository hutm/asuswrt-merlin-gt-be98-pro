#include "subnet.h"
#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
typedef char *(*serialize_fn)(wgn_subnet_rule *,size_t,char *,size_t);
static unsigned cases;
static void run(serialize_fn serialize,wgn_subnet_rule *list,size_t count,size_t capacity,int prefix,int nulls){
 unsigned char buffer[8200],before[8200];memset(buffer,0xa5,sizeof(buffer));memset(buffer,'p',prefix);buffer[prefix]=0;memcpy(before,buffer,sizeof(buffer));
 char *result=serialize(nulls&1?NULL:list,count,nulls&2?NULL:(char *)buffer,capacity);
 printf("%d:",result==(char *)buffer?1:result==NULL?0:-1);for(size_t i=0;i<sizeof(buffer);i++)if(buffer[i]!=before[i])printf("%zu=%02x,",i,buffer[i]);putchar('\n');cases++;
}
int main(int argc,char **argv){assert(argc==2);void *h=dlopen(argv[1],RTLD_LAZY|RTLD_LOCAL);assert(h);serialize_fn serialize=dlsym(h,"wgn_subnet_list_set_to_buffer");assert(serialize);
 size_t capacities[]={0,1,2,3,15,16,31,32,33,63,64,128,255,512,1024,8191,8192,8193};
 wgn_subnet_rule rules[3];memset(rules,0,sizeof(rules));
 size_t offsets[]={offsetof(wgn_subnet_rule,ipaddr),offsetof(wgn_subnet_rule,ipmask),offsetof(wgn_subnet_rule,dhcp_start),offsetof(wgn_subnet_rule,dhcp_end),offsetof(wgn_subnet_rule,domain_name),offsetof(wgn_subnet_rule,dns),offsetof(wgn_subnet_rule,wins),offsetof(wgn_subnet_rule,macipbinding)};
 size_t limits[]={16,16,16,16,132,132,132,1024};unsigned lengths[]={0,1,15,16,30,31,32,33,131,132,1023,1024};
 for(unsigned f=0;f<8;f++)for(unsigned len=0;len<12;len++){
  if(lengths[len]>limits[f])continue;
  memset(rules,0,sizeof(rules));for(unsigned r=0;r<3;r++){
   char *field=(char *)&rules[r]+offsets[f];memset(field,'a'+r,lengths[len]);field[lengths[len]]=0;rules[r].dhcp_enable=r;rules[r].dhcp_lease=r?UINT32_MAX:0x80000000;rules[r].ema=0x7fffffff;rules[r].type=1;
  }
  for(unsigned c=0;c<18;c++)for(size_t count=0;count<4;count++)for(int prefix=0;prefix<=35;prefix+=35)run(serialize,rules,count,capacities[c],prefix,0);
 }
 for(unsigned c=0;c<18;c++)for(int nulls=0;nulls<4;nulls++)run(serialize,rules,1,capacities[c],0,nulls);
 fprintf(stderr,"%u subnet serializer comparisons\n",cases);return 0;
}
