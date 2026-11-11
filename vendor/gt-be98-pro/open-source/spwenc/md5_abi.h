/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef OPEN_SPWENC_MD5_ABI_H
#define OPEN_SPWENC_MD5_ABI_H
#include <stdint.h>
struct nv_md5_context {uint32_t state[4],bits[2];unsigned char buffer[64];};
void MD5Init(struct nv_md5_context *);
void MD5Update(struct nv_md5_context *,const unsigned char *,unsigned int);
void MD5Final(unsigned char digest[16],struct nv_md5_context *);
#endif
