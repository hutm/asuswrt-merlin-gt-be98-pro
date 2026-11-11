/* SPDX-License-Identifier: GPL-2.0-only */
/* Synthetic settings and intercepted AES; no router access. */
#include <dlfcn.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
char *nvram_get(const char *name){(void)name;return "SYNTHETIC";}
char *nvram_get_salt(void){return "synthetic-salt";}
char *get_lan_hwaddr(void){return "02:00:00:00:00:01";}
void AES_init_ctx(void *ctx,const unsigned char *key){(void)ctx;(void)key;}
void AES_ECB_encrypt_buffer(void *ctx,unsigned char *data,int length){
 (void)ctx;int padded=(length+15)&~15;printf("FRAME %d ",length);
 for(int i=0;i<padded;++i)printf("%02x",data[i]);
 puts("");
}
void AES_ECB_decrypt_buffer(void *ctx,unsigned char *data,int length){(void)ctx;(void)data;(void)length;}
int main(void){
 assert(dlopen("libnvram.so",RTLD_LAZY|RTLD_GLOBAL));void *h=dlopen("libshared.so",RTLD_NOW);if(!h){puts(dlerror());return 1;}
 int (*enc)(const char *,char *,int)=dlsym(h,"pw_enc");assert(enc);
 int (*dec)(const char *,char *,int,int)=dlsym(h,"pw_dec");assert(dec);
 char input[65],output[256],decoded[256];
 for(int n=0;n<65;++n){memset(input,'a',n);input[n]=0;memset(output,0xa5,sizeof output);int r=enc(input,output,0);printf("RESULT %d %d %s\n",n,r,output);
 if(n){int result=dec(output,decoded,sizeof decoded,0);printf("DECODE %d %d %s\n",n,result,decoded);
 output[0]=output[0]=='A'?'B':'A';result=dec(output,decoded,sizeof decoded,0);printf("CORRUPT %d %d %d\n",n,result,!strcmp(decoded,output));}}
 return 0;
}
