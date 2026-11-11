/* SPDX-License-Identifier: GPL-2.0-only */
#include "crypto.h"
#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#ifdef REFERENCE
#include <dlfcn.h>
static void *library;
static void (*init)(void *,const unsigned char *);
static void (*encrypt)(void *,unsigned char *);
static void (*decrypt_block)(void *,unsigned char *);
static int run(const unsigned char *key,const unsigned char *in,unsigned char *out,size_t n,int dec){
 unsigned char ctx[256]={0};init(ctx,key);memmove(out,in,n);
 for(size_t i=0;i<n;i+=16)(dec?decrypt_block:encrypt)(ctx,out+i);
 return 0;
}
#else
#define run nv_aes256_ecb
#endif
int main(void){
#ifdef REFERENCE
 assert(dlopen("libnvram.so",RTLD_LAZY|RTLD_GLOBAL));
 library=dlopen("libshared.so",RTLD_NOW);if(!library){fprintf(stderr,"%s\n",dlerror());return 1;}
 init=dlsym(library,"AES_init_ctx");encrypt=dlsym(library,"AES_ECB_encrypt");decrypt_block=dlsym(library,"AES_ECB_decrypt");assert(init&&encrypt&&decrypt_block);
#endif
 unsigned char key[32],plain[64],cipher[64],back[64];
 const unsigned char expected[16]={0x8e,0xa2,0xb7,0xca,0x51,0x67,0x45,0xbf,0xea,0xfc,0x49,0x90,0x4b,0x49,0x60,0x89};
 for(unsigned i=0;i<32;++i)key[i]=i;
 for(unsigned i=0;i<16;++i)plain[i]=i*17;
 assert(!run(key,plain,cipher,16,0)&&!memcmp(cipher,expected,16));
 assert(!run(key,cipher,back,16,1)&&!memcmp(back,plain,16));
 for(unsigned test=0;test<256;++test){
  for(unsigned i=0;i<32;++i)key[i]=(test*17+i*13)&255;
  for(unsigned i=0;i<64;++i)plain[i]=(test*29+i*7)&255;
  assert(!run(key,plain,cipher,64,0));
  assert(!run(key,cipher,back,64,1)&&!memcmp(back,plain,64));
  memcpy(back,plain,64);assert(!run(key,back,back,64,0)&&!memcmp(back,cipher,64));
  for(unsigned i=0;i<64;++i)printf("%02x",cipher[i]);
  puts("");
 }
#ifndef REFERENCE
 assert(nv_aes256_ecb(NULL,plain,cipher,16,0)==-1&&errno==EINVAL);
 assert(nv_aes256_ecb(key,plain,cipher,15,0)==-1&&errno==EINVAL);
 assert(nv_aes256_ecb(key,NULL,NULL,0,0)==0);
 assert(nv_aes256_ecb(key,plain,cipher,(size_t)INT_MAX+1,0)==-1&&errno==EOVERFLOW);
#endif
 return 0;
}
