/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef OPEN_SPWENC_CRYPTO_H
#define OPEN_SPWENC_CRYPTO_H
#include <stddef.h>
/* AES-256 ECB without padding. Separate input/output or exact in-place use.
 * Length must be a multiple of 16 and fit int. No authentication is provided.
 * This is a primitive candidate, not the vendor context/wrapper ABI. */
int nv_aes256_ecb(const unsigned char key[32],const unsigned char *input,
                 unsigned char *output,size_t length,int decrypt);
#endif
