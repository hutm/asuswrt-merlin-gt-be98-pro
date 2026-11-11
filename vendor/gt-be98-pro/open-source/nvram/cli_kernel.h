/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef OPEN_NVRAM_CLI_KERNEL_H
#define OPEN_NVRAM_CLI_KERNEL_H
#include <stdio.h>
/* argv starts at the command. Return consumed arguments, 0 if not recognized,
 * or -1 on failure. Incremental operations are not rolled back on error. */
int nv_cli_kernel(int argc,char *const argv[],FILE *output);
#endif
