/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef OPEN_SPWENC_FRAME_H
#define OPEN_SPWENC_FRAME_H
#include <stddef.h>
/* Prepare the pre-AES payload: eight lowercase CRC32 hex bytes, text, then
 * zeros to a 16-byte boundary. Empty text produces no payload. No encryption.
 * Buffers must not overlap. On error output is unchanged and *written is zero. */
int nv_pw_frame(const char *input,unsigned char *output,size_t capacity,size_t *written);
/* Validate a decrypted frame before copying text. CRC is not authentication.
 * Requires canonical zero padding. On failure output remains unchanged. */
int nv_pw_unframe(const unsigned char *input,size_t length,char *output,size_t capacity);
#endif
