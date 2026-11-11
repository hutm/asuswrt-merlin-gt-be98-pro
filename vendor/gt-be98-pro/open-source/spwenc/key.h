/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef OPEN_SPWENC_KEY_H
#define OPEN_SPWENC_KEY_H
/* Derive the legacy 32 ASCII-hex key bytes plus NUL. Pass NULL salt to omit it.
 * Product and canonical six-byte MAC are explicit; no settings/hardware reads.
 * Output is unchanged on failure. This preserves a legacy format, not a new KDF. */
int nv_pw_key(const char *salt,const char *product,const char *mac,char output[33]);
#endif
