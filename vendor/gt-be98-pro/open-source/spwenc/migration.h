/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef OPEN_SPWENC_MIGRATION_H
#define OPEN_SPWENC_MIGRATION_H
/* Layout from the platform bcmnvram.h; native pointer widths remain native. */
struct nvram_tuple {
 char *name,*value;
 unsigned short len,type,acc_level,enc;
 struct nvram_tuple *next;
};
int init_enc_nvram(void);
int start_enc_nvram(void);
int start_dec_nvram(void);
#endif
