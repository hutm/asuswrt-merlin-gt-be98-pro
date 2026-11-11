/* SPDX-License-Identifier: GPL-2.0-only */
#include "inputs.h"
#include "key.h"
#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
static unsigned pa,ma,pc,mc,missing,denied,n;
static char trace[32],key[33],borrowed[100];
static void event(char c){assert(n+1<sizeof trace);trace[n++]=c;trace[n]=0;}
static const char *salt_get(void){event('S');if(denied)return (char *)(intptr_t)-1;if(missing)return NULL;memset(borrowed,'s',80);borrowed[80]=0;return borrowed;}
static const char *product_get(void){event('P');if(pc++<pa)return missing?NULL:"";memset(borrowed,'p',90);borrowed[90]=0;return borrowed;}
static const char *mac_get(void){event('M');if(mc++<ma)return missing?NULL:"";strcpy(borrowed,"02:11:22:33:44:55");return borrowed;}
static void delay(unsigned usec){assert(usec==500);event('D');}
#ifdef REFERENCE
#include <dlfcn.h>
char *nvram_get(const char *name){assert(!strcmp(name,"productid"));return (char *)product_get();}
char *nvram_get_salt(void){return (char *)salt_get();}
char *get_lan_hwaddr(void){return (char *)mac_get();}
int usleep(unsigned usec){delay(usec);return 0;}
void AES_init_ctx(void *ctx,const unsigned char *value){(void)ctx;memcpy(key,value,32);key[32]=0;}
void AES_ECB_encrypt_buffer(void *ctx,unsigned char *data,int length){(void)ctx;(void)data;(void)length;}
#endif
int main(void){
#ifdef REFERENCE
 assert(dlopen("libnvram.so",RTLD_LAZY|RTLD_GLOBAL));void *h=dlopen("libshared.so",RTLD_NOW);if(!h){puts(dlerror());return 1;}
 int (*enc)(const char *,char *,int)=dlsym(h,"pw_enc");assert(enc);char ciphertext[128];
#else
 struct nv_pw_input_ops ops={salt_get,product_get,mac_get,delay};
 struct nv_pw_inputs in;
#endif
 for(missing=0;missing<2;++missing)for(pa=0;pa<4;++pa)for(ma=0;ma<4;++ma)for(int salted=0;salted<2;++salted){
  pc=mc=n=0;trace[0]=0;
#ifdef REFERENCE
  assert(enc("synthetic",ciphertext,salted)==0);
#else
  assert(!nv_pw_inputs_load(&ops,salted,&in));
  assert(!nv_pw_key(in.salt,in.product,in.mac,key));
#endif
  printf("%u %u %u %d %s %s\n",missing,pa,ma,salted,trace,key);
 }
#ifndef REFERENCE
 memset(&in,0x5a,sizeof in);struct nv_pw_inputs before=in;
 denied=1;n=0;trace[0]=0;
 assert(nv_pw_inputs_load(&ops,1,&in)==-1&&errno==EACCES);
 assert(!memcmp(&in,&before,sizeof in)&&!strcmp(trace,"S"));
 assert(nv_pw_inputs_load(NULL,1,&in)==-1&&errno==EINVAL);
 assert(nv_pw_inputs_load(&ops,1,NULL)==-1&&errno==EINVAL);
 ops.salt=NULL;
 assert(nv_pw_inputs_load(&ops,1,&in)==-1&&errno==EINVAL);
 assert(!nv_pw_inputs_load(&ops,0,&in));
#endif
 return 0;
}
