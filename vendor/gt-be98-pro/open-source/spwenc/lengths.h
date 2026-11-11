/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef OPEN_SPWENC_LENGTHS_H
#define OPEN_SPWENC_LENGTHS_H
#include <stddef.h>
/* Counts exclude an output terminator. Return -1 with errno on invalid input
 * or an unrepresentable int-sized result. Input strings must be terminated. */
int pw_enc_len(const char *input);
int pw_enc_blen(const char *input);
int pw_dec_len(const char *input);
/* Checked byte-count forms also permit testing without huge allocations. */
int nv_pw_enc_size(size_t length);
int nv_pw_enc_bsize(size_t length);
int nv_pw_dec_size(size_t length);
#endif
