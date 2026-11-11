/* SPDX-License-Identifier: GPL-2.0-only */
#include "pwenc.h"
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/crypto.h>
#include <openssl/evp.h>
extern char *get_lan_hwaddr(void);
extern void gen_random_string_v2(char *,size_t);
extern void toUpperCase(char *);

char *base64(const void *input,int length)
{
    BIO *filter=NULL,*memory=NULL,*chain=NULL;
    BUF_MEM *buf=NULL;
    char *result=NULL;
    if(length<0 || (!input && length)){errno=EINVAL;return NULL;}
    filter=BIO_new(BIO_f_base64());memory=BIO_new(BIO_s_mem());
    if(!filter || !memory){BIO_free(filter);BIO_free(memory);errno=ENOMEM;return NULL;}
    chain=BIO_push(filter,memory);
    if((length && BIO_write(chain,input,length)!=length) || BIO_flush(chain)!=1 ||
       BIO_get_mem_ptr(chain,&buf)<=0 || !buf || buf->length==SIZE_MAX){errno=EIO;goto done;}
    result=malloc(buf->length+1);
    if(!result){errno=ENOMEM;goto done;}
    if(buf->length)memcpy(result,buf->data,buf->length);
    result[buf->length]=0;
done:
    BIO_free_all(chain);
    return result;
}

/* The legacy ABI requires caller storage for ceil(strlen(input)/16)*16 bytes. */
int aes_encrypt(const char *input,const unsigned char *key,unsigned char *output)
{
    EVP_CIPHER_CTX *ctx=NULL;
    unsigned char *plain=NULL,*cipher=NULL,iv[16]={0};
    size_t n,rounded;int produced=0,final=0,ok=0;
    if(!input || !key || !output){errno=EINVAL;return 0;}
    n=strlen(input);
    if(n>(size_t)INT_MAX-15){errno=EOVERFLOW;return 0;}
    rounded=(n+15)&~(size_t)15;
    if(!rounded)return 1;
    plain=calloc(1,rounded);cipher=malloc(rounded+16);ctx=EVP_CIPHER_CTX_new();
    if(!plain || !cipher || !ctx){errno=ENOMEM;goto done;}
    memcpy(plain,input,n);
    if(!EVP_EncryptInit_ex(ctx,EVP_aes_128_cbc(),NULL,key,iv) ||
       !EVP_CIPHER_CTX_set_padding(ctx,0) ||
       !EVP_EncryptUpdate(ctx,cipher,&produced,plain,(int)rounded) ||
       produced!=(int)rounded || !EVP_EncryptFinal_ex(ctx,cipher+produced,&final) || final){errno=EIO;goto done;}
    memcpy(output,cipher,rounded);ok=1;
done:
    EVP_CIPHER_CTX_free(ctx);
    if(plain){OPENSSL_cleanse(plain,rounded);free(plain);}
    if(cipher){OPENSSL_cleanse(cipher,rounded+16);free(cipher);}
    return ok;
}

static int digest(const char *input,unsigned char out[16])
{
    unsigned int n=0;
    if(!EVP_Digest(input,strlen(input),out,&n,EVP_md5(),NULL) || n!=16){errno=EIO;return -1;}
    return 0;
}

char *pwenc(char *input,char *output,int capacity)
{
    unsigned char key[16],cipher[257]={0};
    char *mac,*encoded=NULL;size_t n;int ok=0;
    if(!input || !output || capacity<0){errno=EINVAL;return NULL;}
    if(strlen(input)>255){errno=EOVERFLOW;return NULL;}
    mac=get_lan_hwaddr();if(!mac){errno=EINVAL;return NULL;}
    if(digest(mac,key))goto done;
    if(!aes_encrypt(input,key,cipher))goto done;
    /* Preserve the existing stored format's first-NUL ciphertext truncation. */
    encoded=base64(cipher,(int)strlen((const char *)cipher));
    if(!encoded)goto done;
    if(capacity){n=strlen(encoded);if(n>=(size_t)capacity)n=(size_t)capacity-1;memcpy(output,encoded,n);output[n]=0;}
    ok=1;
done:
    OPENSSL_cleanse(key,sizeof(key));OPENSSL_cleanse(cipher,sizeof(cipher));
    if(encoded){OPENSSL_cleanse(encoded,strlen(encoded));free(encoded);}
    return ok?output:NULL;
}

int gen_ddns_hostname(char *output,int capacity)
{
    char random[33]={0},suffix[5];unsigned char md5[16];size_t n;
    if(capacity<0 || (!output && capacity)){errno=EINVAL;return -1;}
    gen_random_string_v2(random,29);random[28]=0;toUpperCase(random);
    if(digest(random,md5))return -1;
    snprintf(suffix,sizeof(suffix),"%02x%02x",md5[14],md5[15]);
    n=strlen(random);memcpy(random+n,suffix,5);
    if(capacity)snprintf(output,(size_t)capacity,"B%s",random);
    OPENSSL_cleanse(md5,sizeof(md5));
    return 0;
}
