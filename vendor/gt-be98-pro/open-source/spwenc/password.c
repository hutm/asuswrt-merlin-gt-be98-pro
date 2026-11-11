/* SPDX-License-Identifier: GPL-2.0-only */
#include "password.h"
#include "lengths.h"
#include "frame.h"
#include "crypto.h"
#include "key.h"
#include <openssl/evp.h>
#include <openssl/crypto.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
int nv_pw_encrypt(const char *input,const char *salt,const char *product,
                  const char *mac,char *output,size_t capacity) {
    unsigned char *frame=NULL;char *encoded=NULL,key[33]={0};
    size_t written=0;int bytes,chars,result=-1,saved;
    if(!input||!output||!capacity){errno=EINVAL;return -1;}
    if(!*input){output[0]=0;return 0;}
    bytes=pw_enc_len(input);if(bytes<0)return -1;
    chars=pw_enc_blen(input);if(chars<0)return -1;
    if((size_t)chars>=capacity){errno=ENOSPC;return -1;}
    if(nv_pw_key(salt,product,mac,key))return -1;
    frame=malloc((size_t)bytes);encoded=malloc((size_t)chars+1);
    if(!frame||!encoded){errno=ENOMEM;goto done;}
    if(nv_pw_frame(input,frame,(size_t)bytes,&written))goto done;
    if(written!=(size_t)bytes){errno=EPROTO;goto done;}
    if(nv_aes256_ecb((const unsigned char *)key,frame,frame,written,0))goto done;
    if(EVP_EncodeBlock((unsigned char *)encoded,frame,bytes)!=chars){errno=EIO;goto done;}
    memcpy(output,encoded,(size_t)chars+1);result=0;
done:
    saved=errno;
    OPENSSL_cleanse(key,sizeof key);
    if(frame)OPENSSL_clear_free(frame,(size_t)bytes);
    if(encoded)OPENSSL_clear_free(encoded,(size_t)chars+1);
    errno=saved;return result;
}
