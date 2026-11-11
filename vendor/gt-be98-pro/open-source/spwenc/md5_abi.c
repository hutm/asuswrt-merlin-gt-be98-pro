/* SPDX-License-Identifier: GPL-2.0-only */
#define OPENSSL_SUPPRESS_DEPRECATED
#include "md5_abi.h"
#include <openssl/md5.h>
#include <openssl/crypto.h>
#include <stddef.h>
#include <string.h>
_Static_assert(sizeof(struct nv_md5_context)==88,"Legacy MD5 context size");
_Static_assert(offsetof(struct nv_md5_context,buffer)==24,"Legacy MD5 buffer offset");
static void block(struct nv_md5_context *ctx,const unsigned char *input)
{
 MD5_CTX backend={0};
 backend.A=ctx->state[0];backend.B=ctx->state[1];backend.C=ctx->state[2];backend.D=ctx->state[3];
 MD5_Transform(&backend,input);
 ctx->state[0]=backend.A;ctx->state[1]=backend.B;ctx->state[2]=backend.C;ctx->state[3]=backend.D;
 OPENSSL_cleanse(&backend,sizeof backend);
}
void MD5Init(struct nv_md5_context *ctx)
{
 ctx->state[0]=0x67452301;ctx->state[1]=0xefcdab89;ctx->state[2]=0x98badcfe;ctx->state[3]=0x10325476;
 ctx->bits[0]=ctx->bits[1]=0;
}
void MD5Update(struct nv_md5_context *ctx,const unsigned char *input,unsigned int length)
{
 unsigned used=(ctx->bits[0]>>3)&63,space=64-used;
 uint32_t old=ctx->bits[0];ctx->bits[0]+=(uint32_t)length<<3;
 ctx->bits[1]+=(ctx->bits[0]<old)+(length>>29);
 if(length>=space){
  memcpy(ctx->buffer+used,input,space);block(ctx,ctx->buffer);
  input+=space;length-=space;used=0;
  while(length>=64){block(ctx,input);input+=64;length-=64;}
 }
 if(length)memcpy(ctx->buffer+used,input,length);
}
void MD5Final(unsigned char digest[16],struct nv_md5_context *ctx)
{
 unsigned char count[8],padding[64]={0x80};
 for(unsigned i=0;i<8;++i)count[i]=(unsigned char)(ctx->bits[i/4]>>(8*(i%4)));
 unsigned used=(ctx->bits[0]>>3)&63;
 MD5Update(ctx,padding,used<56?56-used:120-used);MD5Update(ctx,count,8);
 for(unsigned i=0;i<16;++i)digest[i]=(unsigned char)(ctx->state[i/4]>>(8*(i%4)));
 OPENSSL_cleanse(ctx,sizeof *ctx);OPENSSL_cleanse(count,sizeof count);
}
