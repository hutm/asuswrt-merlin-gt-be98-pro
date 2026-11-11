/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef OPEN_SPWENC_SETTINGS_H
#define OPEN_SPWENC_SETTINGS_H
#include <stddef.h>
/* Return 0 for empty input, 1 for nonempty input, -1 for failure.
 * Output unchanged on failure. Legacy entry point requires 4096 output bytes. */
int nv_enc_setting(const char *name,const char *input,char *output,size_t capacity);
int enc_nvram(char *name,char *input,char *output);
#endif
