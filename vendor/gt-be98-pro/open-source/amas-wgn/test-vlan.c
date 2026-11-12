#include "vlan.h"
#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
typedef char *(*serialize_fn)(wgn_vlan_rule *,size_t,char *,size_t);
static unsigned cases;
static void run(serialize_fn serialize,wgn_vlan_rule *list,size_t count,size_t capacity,int prefix,int nulls){
 unsigned char buffer[8200],before[8200];memset(buffer,0xa5,sizeof(buffer));memset(buffer,'p',prefix);buffer[prefix]=0;memcpy(before,buffer,sizeof(buffer));
 char *result=serialize(nulls&1?NULL:list,count,nulls&2?NULL:(char *)buffer,capacity);
 printf("%d:",result==(char *)buffer?1:result==NULL?0:-1);for(size_t i=0;i<sizeof(buffer);i++)if(buffer[i]!=before[i])printf("%zu=%02x,",i,buffer[i]);putchar('\n');cases++;
}
int main(int argc,char **argv){assert(argc==2);void *h=dlopen(argv[1],RTLD_LAZY|RTLD_LOCAL);assert(h);serialize_fn serialize=dlsym(h,"wgn_vlan_list_set_to_buffer");assert(serialize);
 size_t capacities[]={0,1,2,3,15,16,31,32,33,63,64,128,255,512,1024,8191,8192,8193};
 wgn_vlan_rule rules[3];memset(rules,0,sizeof(rules));
 size_t offsets[]={offsetof(wgn_vlan_rule,wanportset),offsetof(wgn_vlan_rule,lanportset),offsetof(wgn_vlan_rule,wl2gset),offsetof(wgn_vlan_rule,wl5gset),offsetof(wgn_vlan_rule,subnet_name)};
 size_t limits[]={8,8,32,32,32};unsigned lengths[]={0,1,7,8,15,16,30,31,32,33,131,132};
 for(unsigned f=0;f<5;f++)for(unsigned len=0;len<12;len++){
  if(lengths[len]>limits[f])continue;
  memset(rules,0,sizeof(rules));for(unsigned r=0;r<3;r++){
   char *field=(char *)&rules[r]+offsets[f];memset(field,'a'+r,lengths[len]);field[lengths[len]]=0;rules[r].enable=r;rules[r].vid=r?UINT32_MAX:0x80000000;rules[r].prio=(char)(r?255:128);rules[r].internet=0x7fffffff;rules[r].public_vlan=UINT32_MAX;rules[r].type=1;
  }
  for(unsigned c=0;c<18;c++)for(size_t count=0;count<4;count++)for(int prefix=0;prefix<=35;prefix+=35)run(serialize,rules,count,capacities[c],prefix,0);
 }
 for(unsigned c=0;c<18;c++)for(int nulls=0;nulls<4;nulls++)run(serialize,rules,1,capacities[c],0,nulls);
 for(unsigned prio=0;prio<256;prio++){rules[0].prio=(char)prio;run(serialize,rules,1,8192,0,0);}
 wgn_vlan_rule *(*find)(wgn_vlan_rule *,size_t,char *)=dlsym(h,"wgn_vlan_list_find");assert(find);
 strcpy(rules[0].subnet_name,"192.168.1.1/24");strcpy(rules[1].subnet_name,"192.168.2.1/24");rules[2].subnet_name[0]=0;
 char *queries[]={NULL,"","1","192.168.","192.168.1.1/24","192.168.2","192.168.2.1/24","192.168.2.1/24x","no match"};
 for(unsigned q=0;q<9;q++)for(unsigned count=0;count<=3;count++)for(unsigned null=0;null<2;null++){
  wgn_vlan_rule *p=find(null?NULL:rules,count,queries[q]);printf("find:%td\n",p?p-rules:-1);cases++;
 }
 fprintf(stderr,"%u VLAN serializer/lookup comparisons\n",cases);return 0;
}
