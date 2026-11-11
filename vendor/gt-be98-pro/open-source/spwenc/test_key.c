/* SPDX-License-Identifier: GPL-2.0-only */
#include "key.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
static char salt[80],product[100],mac[18],key[33];
#ifdef REFERENCE
#include <dlfcn.h>
char *nvram_get(const char *name){assert(!strcmp(name,"productid"));return product;}
char *nvram_get_salt(void){return salt;}
char *get_lan_hwaddr(void){return mac;}
void AES_init_ctx(void *ctx,const unsigned char *value){(void)ctx;memcpy(key,value,32);key[32]=0;}
void AES_ECB_encrypt_buffer(void *ctx,unsigned char *data,int length){(void)ctx;(void)data;(void)length;}
#endif
int main(void){
#ifdef REFERENCE
 assert(dlopen("libnvram.so",RTLD_LAZY|RTLD_GLOBAL));void *h=dlopen("libshared.so",RTLD_NOW);if(!h){puts(dlerror());return 1;}
 int (*enc)(const char *,char *,int)=dlsym(h,"pw_enc");assert(enc);char ciphertext[128];
#endif
 for(unsigned test=0;test<256;++test){
  unsigned slen=test%80,plen=1+test%98;
  for(unsigned i=0;i<slen;++i)salt[i]=(char)('a'+(i+test)%26);
  salt[slen]=0;
  for(unsigned i=0;i<plen;++i)product[i]=(char)('A'+(i*3+test)%26);
  product[plen]=0;
  snprintf(mac,sizeof mac,"%02x:%02x:%02x:%02x:%02x:%02x",test,(test*3)&255,(test*5)&255,(test*7)&255,(test*11)&255,(test*13)&255);
  for(int salted=0;salted<2;++salted){
#ifdef REFERENCE
   assert(enc("synthetic",ciphertext,salted)==0);
#else
   assert(!nv_pw_key(salted?salt:NULL,product,mac,key));
#endif
   printf("%u %d %s\n",test,salted,key);
  }
 }
#ifndef REFERENCE
 strcpy(key,"unchanged");assert(nv_pw_key(NULL,"product","invalid",key)==-1&&errno==EINVAL&&!strcmp(key,"unchanged"));
 assert(nv_pw_key(NULL,NULL,mac,key)==-1&&errno==EINVAL);
 assert(nv_pw_key(NULL,"product","gg:00:00:00:00:00",key)==-1&&errno==EINVAL);
 assert(nv_pw_key(NULL,"product",mac,NULL)==-1&&errno==EINVAL);
#endif
 return 0;
}
