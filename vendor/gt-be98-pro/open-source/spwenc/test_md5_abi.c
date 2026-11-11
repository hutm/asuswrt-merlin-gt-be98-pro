/* SPDX-License-Identifier: GPL-2.0-only */
#include "md5_abi.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#ifdef REFERENCE
#include <dlfcn.h>
#endif
static void dump(const void *p,size_t size){const unsigned char *s=p;for(size_t i=0;i<size;++i)printf("%02x",s[i]);puts("");}
int main(void){
#ifdef REFERENCE
 assert(dlopen("libnvram.so",RTLD_LAZY|RTLD_GLOBAL));void *h=dlopen("libshared.so",RTLD_NOW);assert(h);
 void (*init)(struct nv_md5_context *)=dlsym(h,"MD5Init");
 void (*update)(struct nv_md5_context *,const unsigned char *,unsigned int)=dlsym(h,"MD5Update");
 void (*final)(unsigned char *,struct nv_md5_context *)=dlsym(h,"MD5Final");assert(init&&update&&final);
#else
 void (*init)(struct nv_md5_context *)=MD5Init;
 void (*update)(struct nv_md5_context *,const unsigned char *,unsigned int)=MD5Update;
 void (*final)(unsigned char *,struct nv_md5_context *)=MD5Final;
#endif
 unsigned char data[513],digest[16];for(unsigned i=0;i<sizeof data;++i)data[i]=(unsigned char)(i*37);
 unsigned chunks[]={1,7,55,56,63,64,65,128,513};
 for(unsigned length=0;length<=513;++length)for(unsigned c=0;c<9;++c){
  struct nv_md5_context ctx;memset(&ctx,0xa5,sizeof ctx);init(&ctx);
  for(unsigned pos=0;pos<length;){unsigned n=length-pos;if(n>chunks[c])n=chunks[c];update(&ctx,data+pos,n);pos+=n;}
  update(&ctx,NULL,0);printf("C %u %u ",length,chunks[c]);dump(&ctx,sizeof ctx);
  final(digest,&ctx);printf("D ");dump(digest,sizeof digest);
  for(unsigned i=0;i<sizeof ctx;++i)assert(((unsigned char *)&ctx)[i]==0);
 }
 struct nv_md5_context ctx;memset(&ctx,0xa5,sizeof ctx);init(&ctx);ctx.bits[0]=0xfffffff8;ctx.bits[1]=0xffffffff;
 update(&ctx,data,65);dump(&ctx,sizeof ctx);final(digest,&ctx);dump(digest,sizeof digest);
 return 0;
}
