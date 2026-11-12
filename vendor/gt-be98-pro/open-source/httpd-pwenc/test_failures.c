#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include "pwenc.h"
static int malloc_fail,calloc_fail,ctx_fail,digest_fail,bio_fail,bio_count;
void *__real_malloc(size_t);
void *__real_calloc(size_t,size_t);
EVP_CIPHER_CTX *__real_EVP_CIPHER_CTX_new(void);
BIO *__real_BIO_new(const BIO_METHOD *);
int __real_EVP_Digest(const void *,size_t,unsigned char *,unsigned int *,const EVP_MD *,ENGINE *);
void *__wrap_malloc(size_t n){if(malloc_fail){malloc_fail--;return NULL;}return __real_malloc(n);}
void *__wrap_calloc(size_t n,size_t s){if(calloc_fail){calloc_fail--;return NULL;}return __real_calloc(n,s);}
EVP_CIPHER_CTX *__wrap_EVP_CIPHER_CTX_new(void){return ctx_fail?NULL:__real_EVP_CIPHER_CTX_new();}
BIO *__wrap_BIO_new(const BIO_METHOD *method){bio_count++;return bio_fail==bio_count?NULL:__real_BIO_new(method);}
int __wrap_EVP_Digest(const void *in,size_t n,unsigned char *out,unsigned int *len,const EVP_MD *type,ENGINE *impl){return digest_fail?0:__real_EVP_Digest(in,n,out,len,type,impl);}
char *get_lan_hwaddr(void){return "02:11:22:33:44:55";}
void gen_random_string_v2(char *out,size_t n){assert(n==29);strcpy(out,"abcd1234");}
void toUpperCase(char *out){(void)out;}
static void unchanged(const unsigned char *p,size_t n){for(size_t i=0;i<n;i++)assert(p[i]==0x5a);}
int main(void){
 unsigned char out[512],key[16]={0};memset(out,0x5a,sizeof(out));
 malloc_fail=1;assert(!aes_encrypt("abc",key,out)&&errno==ENOMEM);unchanged(out,sizeof(out));
 calloc_fail=1;assert(!aes_encrypt("abc",key,out)&&errno==ENOMEM);unchanged(out,sizeof(out));
 ctx_fail=1;assert(!aes_encrypt("abc",key,out)&&errno==ENOMEM);unchanged(out,sizeof(out));ctx_fail=0;
 for(int i=1;i<=2;i++){bio_fail=i;bio_count=0;assert(!base64("abc",3)&&errno==ENOMEM);}bio_fail=0;
 malloc_fail=1;assert(!base64("abc",3)&&errno==ENOMEM);
 digest_fail=1;assert(!pwenc("abc",(char *)out,sizeof(out))&&errno==EIO);unchanged(out,sizeof(out));assert(gen_ddns_hostname((char *)out,sizeof(out))==-1&&errno==EIO);unchanged(out,sizeof(out));digest_fail=0;
 ctx_fail=1;assert(!pwenc("abc",(char *)out,sizeof(out))&&errno==ENOMEM);unchanged(out,sizeof(out));ctx_fail=0;
 bio_fail=1;bio_count=0;assert(!pwenc("abc",(char *)out,sizeof(out))&&errno==ENOMEM);unchanged(out,sizeof(out));bio_fail=0;
 return 0;
}
