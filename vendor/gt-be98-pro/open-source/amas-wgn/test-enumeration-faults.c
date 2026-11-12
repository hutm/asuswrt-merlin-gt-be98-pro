#include <assert.h>
#include <dlfcn.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static unsigned allocations,fail_at,outstanding;
void *wgn_test_malloc(size_t n){if(++allocations==fail_at){errno=ENOMEM;return NULL;}void *p=malloc(n);if(p)outstanding++;return p;}
void wgn_test_free(void *p){if(p){assert(outstanding);outstanding--;}free(p);}
char *wgn_test_strdup(const char *s){size_t n=strlen(s)+1;char *p=wgn_test_malloc(n);if(p)memcpy(p,s,n);return p;}
char *nvram_get(const char *key){assert(!strcmp(key,"vlan_rulelist"));return "<1>20>3>a>b>0002>0000>sub>1>0>1";}
int main(int argc,char **argv){assert(argc==2);void *h=dlopen(argv[1],RTLD_LAZY|RTLD_LOCAL);assert(h);
 char *(*one)(int,int,char *,size_t)=dlsym(h,"wgn_guest_ifnames");char *(*all)(char *,size_t)=dlsym(h,"wgn_guest_all_ifnames");char *(*vlans)(char *,char *,size_t)=dlsym(h,"wgn_guest_vlans");void (*subnet)(const char *,char *,int)=dlsym(h,"wgn_subnet");assert(one&&all&&vlans&&subnet);
 for(unsigned which=0;which<4;which++)for(fail_at=1;fail_at<=3;fail_at++){
  unsigned char out[64],expected[64];memset(out,0xa5,sizeof(out));out[0]=0;memcpy(expected,out,sizeof(out));allocations=0;errno=0;
  if(which==0)assert(!one(0,0,(char *)out,sizeof(out)));
  if(which==1)assert(!all((char *)out,sizeof(out)));
  if(which==2)assert(!vlans("wl0.1",(char *)out,sizeof(out)));
  if(which==3)subnet("wl0.1",(char *)out,sizeof(out));
  if((which==0||which==2)&&fail_at>1)memset(expected,0,sizeof(expected));
  assert(!memcmp(out,expected,sizeof(out)));assert(allocations==fail_at&&!outstanding&&errno==ENOMEM);
 }
 puts("12 enumeration allocation-failure cases passed");
}
