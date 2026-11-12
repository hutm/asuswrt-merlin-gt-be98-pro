#include "vlan.h"
#include <dlfcn.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
static int fail,calls;
char *strdup(const char *input){if(fail){calls++;errno=ENOMEM;return NULL;}size_t n=strlen(input)+1;char *p=malloc(n);if(p)memcpy(p,input,n);return p;}
int main(int argc,char **argv){assert(argc==2);void *h=dlopen(argv[1],RTLD_LAZY|RTLD_LOCAL);assert(h);
 wgn_vlan_rule *(*parse)(char *,wgn_vlan_rule *,size_t,size_t *,int)=dlsym(h,"wgn_vlan_list_get_from_content");assert(parse);
 char input[]="<192.168.1.1>255.255.255.0>1>a>b>2>c>d>e>3>f>1";
 for(int nulls=0;nulls<8;nulls++)for(int capacity=0;capacity<2;capacity++){
  wgn_vlan_rule list[2],before[2];memset(list,0x5a,sizeof(list));memcpy(before,list,sizeof(list));size_t count=99;calls=0;errno=0;fail=1;
  assert(parse(nulls&1?NULL:input,nulls&2?NULL:list,capacity,nulls&4?NULL:&count,0)==NULL);int error=errno;fail=0;
  assert(!memcmp(list,before,sizeof(list)));if(!(nulls&4))assert(count==0);
  int allocated=!(nulls&3)&&capacity;assert(calls==allocated);assert(error==(allocated?ENOMEM:0));
  printf("%d:%d:%d:%zu\n",nulls,capacity,calls,count);
 }
 fprintf(stderr,"16 allocation and invalid-input cases passed\n");return 0;
}
