/* SPDX-License-Identifier: GPL-2.0-only */
#include "aes_abi.h"
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
 void (*init)(struct nv_aes_context *,const unsigned char *)=dlsym(h,"AES_init_ctx");
 void (*enc)(const struct nv_aes_context *,unsigned char *,int)=dlsym(h,"AES_ECB_encrypt_buffer"),(*dec)(const struct nv_aes_context *,unsigned char *,int)=dlsym(h,"AES_ECB_decrypt_buffer");
 void (*enc1)(const struct nv_aes_context *,unsigned char *)=dlsym(h,"AES_ECB_encrypt"),(*dec1)(const struct nv_aes_context *,unsigned char *)=dlsym(h,"AES_ECB_decrypt");assert(init&&enc&&dec&&enc1&&dec1);
#else
 void (*init)(struct nv_aes_context *,const unsigned char *)=AES_init_ctx;
 void (*enc)(const struct nv_aes_context *,unsigned char *,int)=AES_ECB_encrypt_buffer,(*dec)(const struct nv_aes_context *,unsigned char *,int)=AES_ECB_decrypt_buffer;
 void (*enc1)(const struct nv_aes_context *,unsigned char *)=AES_ECB_encrypt,(*dec1)(const struct nv_aes_context *,unsigned char *)=AES_ECB_decrypt;
#endif
 unsigned char key[32],data[96],plain[96];struct nv_aes_context ctx;
 for(unsigned k=0;k<64;++k){
  for(unsigned i=0;i<32;++i)key[i]=(unsigned char)(k*17+i*7);
  init(&ctx,key);printf("K %u ",k);dump(&ctx,sizeof ctx);struct nv_aes_context copy=ctx;
  for(int length=-1;length<=65;++length){
   for(unsigned i=0;i<sizeof data;++i)plain[i]=data[i]=(unsigned char)(i*11+k);
   enc(&copy,data,length);printf("E %d ",length);dump(data,sizeof data);
   if(length>0){unsigned rounded=((unsigned)length+15)&~15U;dec(&copy,data,(int)rounded);assert(!memcmp(data,plain,(size_t)length));for(unsigned i=(unsigned)length;i<rounded;++i)assert(!data[i]);}
   memcpy(data,plain,sizeof data);dec(&copy,data,length);printf("D %d ",length);dump(data,sizeof data);
  }
  memcpy(data,plain,sizeof data);enc1(&copy,data);printf("B ");dump(data,16);dec1(&copy,data);assert(!memcmp(data,plain,16));assert(!memcmp(&ctx,&copy,sizeof ctx));
 }
 return 0;
}
