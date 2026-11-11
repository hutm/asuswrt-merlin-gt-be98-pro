/* SPDX-License-Identifier: GPL-2.0-only */
#include "aes_abi.h"
#include "crypto.h"
#include <errno.h>
#include <string.h>
/* Algebraic AES substitution avoids a separate table dependency. */
static unsigned char multiply(unsigned char a,unsigned char b)
{
 unsigned char out=0;
 for(unsigned i=0;i<8;++i){out^=a*(b&1);a=(unsigned char)((a<<1)^((a>>7)*0x1b));b>>=1;}
 return out;
}
static unsigned char rotate(unsigned char value,unsigned n)
{return (unsigned char)((value<<n)|(value>>(8-n)));}
static unsigned char substitute(unsigned char value)
{
 unsigned char inverse=1,power=value;
 for(unsigned exponent=254;exponent;exponent>>=1){if(exponent&1)inverse=multiply(inverse,power);power=multiply(power,power);}
 return inverse^rotate(inverse,1)^rotate(inverse,2)^rotate(inverse,3)^rotate(inverse,4)^0x63;
}
void AES_init_ctx(struct nv_aes_context *ctx,const unsigned char key[32])
{
 if(!ctx||!key){errno=EINVAL;return;}
 memcpy(ctx->round_key,key,32);unsigned char rcon=1;
 for(unsigned word=8;word<60;++word){
  unsigned char temp[4];memcpy(temp,ctx->round_key+4*(word-1),4);
  if(word%8==0){unsigned char first=temp[0];for(unsigned i=0;i<3;++i)temp[i]=substitute(temp[i+1]);temp[3]=substitute(first);temp[0]^=rcon;rcon=multiply(rcon,2);}
  else if(word%8==4)for(unsigned i=0;i<4;++i)temp[i]=substitute(temp[i]);
  for(unsigned i=0;i<4;++i)ctx->round_key[4*word+i]=ctx->round_key[4*(word-8)+i]^temp[i];
 }
}
static void crypt(const struct nv_aes_context *ctx,unsigned char *data,int length,int decrypt)
{
 if(length<=0)return;
 if(!ctx||!data){errno=EINVAL;return;}
 size_t n=(size_t)length,rounded=(n+15)&~(size_t)15;
 if(rounded!=n)memset(data+n,0,rounded-n);
 (void)nv_aes256_ecb(ctx->round_key,data,data,rounded,decrypt);
}
void AES_ECB_encrypt(const struct nv_aes_context *ctx,unsigned char data[16]){crypt(ctx,data,16,0);}
void AES_ECB_decrypt(const struct nv_aes_context *ctx,unsigned char data[16]){crypt(ctx,data,16,1);}
void AES_ECB_encrypt_buffer(const struct nv_aes_context *ctx,unsigned char *data,int length){crypt(ctx,data,length,0);}
void AES_ECB_decrypt_buffer(const struct nv_aes_context *ctx,unsigned char *data,int length){crypt(ctx,data,length,1);}
