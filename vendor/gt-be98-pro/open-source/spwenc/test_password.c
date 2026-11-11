/* SPDX-License-Identifier: GPL-2.0-only */
#include "password.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
static char salt[80],product[100],mac[18];
#ifdef REFERENCE
#include <dlfcn.h>
char *nvram_get(const char *name){assert(!strcmp(name,"productid"));return product;}
char *nvram_get_salt(void){return salt;}
char *get_lan_hwaddr(void){return mac;}
#endif
int main(void){
#ifdef REFERENCE
 assert(dlopen("libnvram.so",RTLD_LAZY|RTLD_GLOBAL));void *h=dlopen("libshared.so",RTLD_NOW);if(!h){puts(dlerror());return 1;}
 int (*enc)(const char *,char *,int)=dlsym(h,"pw_enc");
 int (*dec)(const char *,char *,int,int)=dlsym(h,"pw_dec");assert(enc&&dec);
 #endif
 char decoded[1024];
 char input[513],output[1024];
 for(unsigned test=0;test<=512;++test){
  unsigned slen=test%80,plen=1+test%98;
  for(unsigned i=0;i<slen;++i)salt[i]=(char)('a'+(i+test)%26);
  salt[slen]=0;
  for(unsigned i=0;i<plen;++i)product[i]=(char)('A'+(i*3+test)%26);
  product[plen]=0;
  snprintf(mac,sizeof mac,"%02x:%02x:%02x:%02x:%02x:%02x",test&255,(test*3)&255,(test*5)&255,(test*7)&255,(test*11)&255,(test*13)&255);
  for(unsigned i=0;i<test;++i)input[i]=(char)(32+(i*7+test)%95);
  input[test]=0;
  for(int salted=0;salted<2;++salted){
   memset(output,0xa5,sizeof output);
#ifdef REFERENCE
   assert(enc(input,output,salted)==0);
   assert(dec(output,decoded,sizeof decoded,salted)==(test?1:0));assert(!strcmp(decoded,input));
#else
   assert(!nv_pw_encrypt(input,salted?salt:NULL,product,mac,output,sizeof output));
   assert(!nv_pw_decrypt(output,salted?salt:NULL,product,mac,decoded,sizeof decoded)&&!strcmp(decoded,input));
   assert(!nv_pw_decrypt(output,salted?salt:NULL,product,mac,decoded,test+1)&&!strcmp(decoded,input));
   if(test){strcpy(decoded,"unchanged");assert(nv_pw_decrypt(output,salted?salt:NULL,product,mac,decoded,test)==-1&&errno==ENOSPC&&!strcmp(decoded,"unchanged"));}
#endif
   printf("%u %d %s\n",test,salted,output);
  }
 }
#ifndef REFERENCE
 strcpy(output,"unchanged");
 assert(nv_pw_encrypt("secret",salt,product,mac,output,1)==-1&&errno==ENOSPC&&!strcmp(output,"unchanged"));
 assert(nv_pw_encrypt("secret",salt,product,"bad",output,sizeof output)==-1&&errno==EINVAL&&!strcmp(output,"unchanged"));
 assert(nv_pw_encrypt(NULL,salt,product,mac,output,sizeof output)==-1&&errno==EINVAL);
 assert(nv_pw_encrypt("secret",salt,product,mac,NULL,16)==-1&&errno==EINVAL);
 assert(!nv_pw_encrypt("",NULL,NULL,NULL,output,1)&&!output[0]);
 const char *bad[]={"A","AAAA","====","!!!!","AA=A","AAAA\n","AAAAAAAAAAAAAAAAAAAAAB=="};
 for(unsigned i=0;i<sizeof bad/sizeof bad[0];++i){strcpy(decoded,"unchanged");assert(nv_pw_decrypt(bad[i],salt,product,mac,decoded,sizeof decoded)==-1&&errno==EPROTO&&!strcmp(decoded,"unchanged"));}
 assert(!nv_pw_encrypt("secret",salt,product,mac,output,sizeof output));strcpy(decoded,"unchanged");
 assert(nv_pw_decrypt(output,"wrong-salt",product,mac,decoded,sizeof decoded)==-1&&!strcmp(decoded,"unchanged"));
 output[0]=output[0]=='A'?'B':'A';assert(nv_pw_decrypt(output,salt,product,mac,decoded,sizeof decoded)==-1&&!strcmp(decoded,"unchanged"));
 assert(nv_pw_decrypt(NULL,salt,product,mac,decoded,sizeof decoded)==-1&&errno==EINVAL);
 assert(!nv_pw_decrypt("",NULL,NULL,NULL,decoded,1)&&!decoded[0]);
#endif
 return 0;
}
