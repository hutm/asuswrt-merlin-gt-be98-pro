/* SPDX-License-Identifier: GPL-2.0-only */
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include "pwenc.h"
static unsigned int scenario;
static char mac[18];
size_t strlcpy(char *d,const char *s,size_t n){size_t k=strlen(s);if(n){size_t c=k<n-1?k:n-1;memcpy(d,s,c);d[c]=0;}return k;}
size_t strlcat(char *d,const char *s,size_t n){size_t k=strnlen(d,n);if(k==n)return n+strlen(s);return k+strlcpy(d+k,s,n-k);}
char *get_lan_hwaddr(void){return mac;}
void gen_random_string_v2(char *s,size_t n){assert(n==29);size_t k=scenario%29;for(size_t i=0;i<k;i++)s[i]="aBcD019xyz"[(scenario+i)%10];s[k]=0;}
void toUpperCase(char *s){for(;*s;s++)*s=(char)toupper((unsigned char)*s);}
static void md5(const char *s,unsigned char out[16]){unsigned int n=0;assert(EVP_Digest(s,strlen(s),out,&n,EVP_md5(),NULL));assert(n==16);}
static size_t encrypt(const char *s,const unsigned char key[16],unsigned char *out){
 unsigned char in[272]={0},iv[16]={0};size_t n=strlen(s),r=(n+15)&~(size_t)15;int len=0,tail=0;
 memcpy(in,s,n);EVP_CIPHER_CTX *ctx=EVP_CIPHER_CTX_new();assert(ctx);
 assert(EVP_EncryptInit_ex(ctx,EVP_aes_128_cbc(),NULL,key,iv));assert(EVP_CIPHER_CTX_set_padding(ctx,0));
 assert(EVP_EncryptUpdate(ctx,out,&len,in,(int)r));assert(EVP_EncryptFinal_ex(ctx,out+len,&tail));assert((size_t)(len+tail)==r);EVP_CIPHER_CTX_free(ctx);return r;
}
static void encoded(const unsigned char *in,int n,char out[1024]){
 unsigned char flat[1024];int len=EVP_EncodeBlock(flat,in,n);assert(len>=0);size_t pos=0;
 for(int i=0;i<len;i++){out[pos++]=(char)flat[i];if(i%64==63)out[pos++]='\n';}
 if(len%64)out[pos++]='\n';
 out[pos]=0;
}
static void emit(const char *kind,int i,int cap,const void *buf,size_t n){printf("CHECK %s %d %d ",kind,i,cap);for(size_t j=0;j<n;j++)printf("%02x",((const unsigned char *)buf)[j]);putchar('\n');}
int main(void){
 unsigned char key[16],actual[288],expected[288],digest[16],bytes[513];char input[257],want[1024],out[1024];
 for(int n=0;n<=512;n++){for(int i=0;i<n;i++)bytes[i]=(unsigned char)(i*137+n);encoded(bytes,n,want);char *s=base64(bytes,n);assert(s&&!strcmp(s,want));emit("base64",n,0,s,strlen(s));free(s);}
 for(int k=0;k<8;k++)for(int n=0;n<=255;n++){
  for(int i=0;i<16;i++)key[i]=(unsigned char)(k*17+i*13);
  for(int i=0;i<n;i++)input[i]=(char)(1+(i*137+k)%255);
  input[n]=0;memset(actual,0x5a,sizeof(actual));memset(expected,0x5a,sizeof(expected));
  size_t r=encrypt(input,key,expected);assert(aes_encrypt(input,key,actual)==1);assert(!memcmp(actual,expected,sizeof(actual)));emit("aes",k,n,actual,r);
 }
 static const int caps[]={0,1,2,16,32,33,34,64,384};
 for(scenario=0;scenario<58;scenario++){
  char random[33]={0},name[40];gen_random_string_v2(random,29);toUpperCase(random);md5(random,digest);
  snprintf(name,sizeof(name),"B%s%02x%02x",random,digest[14],digest[15]);
  for(size_t c=0;c<sizeof(caps)/sizeof(caps[0]);c++){memset(out,0x5a,sizeof(out));memset(want,0x5a,sizeof(want));if(caps[c])snprintf(want,(size_t)caps[c],"%s",name);assert(!gen_ddns_hostname(out,caps[c]));assert(!memcmp(out,want,sizeof(out)));emit("hostname",(int)scenario,caps[c],out,40);}
 }
 for(int k=0;k<8;k++)for(int n=0;n<=239;n++){
  snprintf(mac,sizeof(mac),"02:11:22:33:44:%02x",k);md5(mac,key);
  for(int i=0;i<n;i++)input[i]=(char)(1+(k*19+i*137)%255);
  input[n]=0;memset(expected,0,sizeof(expected));encrypt(input,key,expected);encoded(expected,(int)strlen((char *)expected),want);
  for(size_t c=0;c<sizeof(caps)/sizeof(caps[0]);c++){memset(out,0x5a,sizeof(out));char copy[1024];memset(copy,0x5a,sizeof(copy));if(caps[c])snprintf(copy,(size_t)caps[c],"%s",want);assert(pwenc(input,out,caps[c])==out);assert(!memcmp(copy,out,sizeof(out)));emit("password",k*240+n,caps[c],out,400);}
 }
#ifndef REFERENCE
 assert(!base64(NULL,1)&&errno==EINVAL);assert(!base64("x",-1)&&errno==EINVAL);
 assert(!aes_encrypt(NULL,key,actual));assert(!aes_encrypt("x",NULL,actual));assert(!aes_encrypt("x",key,NULL));
 memset(input,'x',256);input[256]=0;memset(out,0x5a,sizeof(out));assert(!pwenc(input,out,sizeof(out))&&errno==EOVERFLOW);for(size_t i=0;i<sizeof(out);i++)assert(out[i]==0x5a);
 assert(!pwenc(NULL,out,1));assert(!pwenc("x",NULL,1));assert(!pwenc("x",out,-1));
 assert(gen_ddns_hostname(NULL,1)==-1);assert(gen_ddns_hostname(out,-1)==-1);assert(!gen_ddns_hostname(NULL,0));
#endif
 return 0;
}
