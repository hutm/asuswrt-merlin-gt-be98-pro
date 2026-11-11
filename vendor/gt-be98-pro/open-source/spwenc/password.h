/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef OPEN_SPWENC_PASSWORD_H
#define OPEN_SPWENC_PASSWORD_H
#include <stddef.h>
/* Complete legacy encryption with explicit device inputs and output capacity.
 * NULL salt omits it. Empty plaintext produces an empty string. Output remains
 * unchanged on failure. Input/output buffers must not overlap. */
int nv_pw_encrypt(const char *input,const char *salt,const char *product,
                  const char *mac,char *output,size_t capacity);
#endif
