/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef OPEN_SPWENC_WRAPPER_H
#define OPEN_SPWENC_WRAPPER_H
/* Legacy ABI: caller provides pw_enc_blen(input)+1 bytes for encryption.
 * Decoder returns 1 for plaintext, 0 for empty/pass-through, -1 for errors.
 * Decoder len is enforced as destination capacity, including the terminator. */
int pw_enc(const char *input,char *output,int with_salt);
int pw_dec(const char *input,char *output,int len,int with_salt);
#endif
