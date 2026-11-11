/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef OPEN_NVRAM_MANUFACTURING_H
#define OPEN_NVRAM_MANUFACTURING_H
#include <stdio.h>
/* Append newline records from a NUL-separated manufacturing file. Empty record
 * terminates input. Optional base_mac is read only if no et0macaddr= record was
 * present. Errors may follow partial output; caller owns and closes streams. */
int nv_mfg_append(FILE *input,FILE *output,FILE *base_mac);
/* Append to destination and flush/fsync it. Optional missing base_mac is ignored.
 * No rollback on failure; concurrent callers must serialize destination writes. */
int nv_mfg_restore(const char *source,const char *destination,const char *base_mac);
#endif
