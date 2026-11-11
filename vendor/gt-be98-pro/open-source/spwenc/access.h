/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef OPEN_SPWENC_ACCESS_H
#define OPEN_SPWENC_ACCESS_H
struct INVALID_NVRAM_GET_TABLE { char *name,*exclude_process; };
extern struct INVALID_NVRAM_GET_TABLE invalid_nvram_get_t[];
int invalid_nvram_get_name(char *name);
int invalid_program_check(void);
int invalid_nvram_get_program(char *name);
#endif
