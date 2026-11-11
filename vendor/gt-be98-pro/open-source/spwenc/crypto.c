/* SPDX-License-Identifier: GPL-2.0-only */
#include "crypto.h"
#include <openssl/evp.h>
#include <errno.h>
#include <limits.h>
int nv_aes256_ecb(const unsigned char key[32],const unsigned char *input,
                 unsigned char *output,size_t length,int decrypt) {
    EVP_CIPHER_CTX *ctx;int produced=0,final=0,ok;
    if(!key || (length && (!input || !output)) || length%16 || (decrypt!=0 && decrypt!=1)) {
        errno=EINVAL;return -1;
    }
    if(length>INT_MAX) {errno=EOVERFLOW;return -1;}
    if(!length) return 0;
    ctx=EVP_CIPHER_CTX_new();if(!ctx) {errno=ENOMEM;return -1;}
    ok=EVP_CipherInit_ex(ctx,EVP_aes_256_ecb(),NULL,key,NULL,!decrypt) &&
       EVP_CIPHER_CTX_set_padding(ctx,0) &&
       EVP_CipherUpdate(ctx,output,&produced,input,(int)length) &&
       produced==(int)length && EVP_CipherFinal_ex(ctx,output+produced,&final) && final==0;
    EVP_CIPHER_CTX_free(ctx);
    if(!ok) {errno=EIO;return -1;}
    return 0;
}
