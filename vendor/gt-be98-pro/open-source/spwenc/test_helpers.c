/* SPDX-License-Identifier: GPL-2.0-only */
#include "helpers.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
static const char *setting;
char *nvram_get(const char *name){(void)name;return (char *)setting;}
#ifdef REFERENCE
#include <dlfcn.h>
#endif
int main(void){
#ifdef REFERENCE
 assert(dlopen("libnvram.so",RTLD_LAZY|RTLD_GLOBAL));void *h=dlopen("libshared.so",RTLD_NOW);assert(h);
 char *(*hash)(const char *,int,char *)=dlsym(h,"str_to_md5");
 int (*same)(const char *,const char *,const char *)=dlsym(h,"is_same_pass");
 void (*get)(const char *,const char *,char *,int)=dlsym(h,"get_encrypt_list_value");assert(hash&&same&&get);
#else
 char *(*hash)(const char *,int,char *)=str_to_md5;
 int (*same)(const char *,const char *,const char *)=is_same_pass;
 void (*get)(const char *,const char *,char *,int)=get_encrypt_list_value;
#endif
 char data[512],digest[33],out[40];
 for(unsigned i=0;i<sizeof data;++i)data[i]=(char)(i*37);
 for(int n=0;n<=512;++n){assert(hash(data,n,digest)==digest);printf("H %d %s\n",n,digest);}
 const char *lists[]={NULL,"","broken","<a>pass","<a>pass<a>next","<a><>pass","a>pass>extra<b>next","<<a>pass<<b>next<",">","a>>pass"};
 const char *names[]={"acc_list","pptpd_clientlist","vpn_serverx_clientlist","custom"};
 const char *users[]={"","a","b","missing"};
 const char *passwords[]={"","pass","next","1a1dc91c907325c69271ddf0c944bc72","d41d8cd98f00b204e9800998ecf8427e"};
 for(unsigned l=0;l<10;++l)for(unsigned n=0;n<4;++n)for(unsigned u=0;u<4;++u){
  setting=lists[l];
  for(int cap=0;cap<8;++cap){memset(out,'X',sizeof out);out[sizeof out-1]=0;get(names[n],users[u],out,cap);printf("G %u %u %u %d %s\n",l,n,u,cap,out);}
  for(unsigned p=0;p<5;++p)printf("S %u %u %u %u %d\n",l,n,u,p,same(names[n],users[u],passwords[p]));
 }
#ifndef REFERENCE
 strcpy(out,"unchanged");assert(!hash(data,-1,out)&&errno==EINVAL&&!strcmp(out,"unchanged"));
 assert(!hash(NULL,0,out)&&errno==EINVAL);assert(!hash(data,0,NULL)&&errno==EINVAL);
 assert(!same(NULL,"a","pass")&&errno==EINVAL);
#endif
 return 0;
}
