/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef OPEN_NVRAM_PROFILE_H
#define OPEN_NVRAM_PROFILE_H
#include <stddef.h>
#include <stdio.h>
/* HDR1/HDR2 backup reader. Validate the complete bounded payload before applying.
 * NULL values mean unset. Callback errors stop with possible partial changes. */
int nv_profile_read(FILE *input,size_t capacity,int (*apply)(void *,const char *,const char *),void *context);
/* Write an HDR2 profile from a bounded, double-NUL-terminated settings list.
 * salt is 0..29; it is format obfuscation, not encryption. Input is unchanged.
 * Caller owns the stream and must check close errors. */
int nv_profile_write(FILE *output,const char *settings,size_t capacity,unsigned salt);
#endif
