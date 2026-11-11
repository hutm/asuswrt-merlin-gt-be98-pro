/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef OPEN_NVRAM_PROFILE_H
#define OPEN_NVRAM_PROFILE_H
#include <stddef.h>
#include <stdio.h>
/* HDR1/HDR2 backup reader. Validate the complete bounded payload before applying.
 * NULL values mean unset. Callback errors stop with possible partial changes. */
int nv_profile_read(FILE *input,size_t capacity,int (*apply)(void *,const char *,const char *),void *context);
#endif
