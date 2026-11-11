/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef OPEN_NVRAM_RUNTIME_H
#define OPEN_NVRAM_RUNTIME_H
/* Experimental subset of the vendor ABI. Returned strings are borrowed and
 * may change after later calls. Not async-signal-safe. No library unloading
 * while callbacks/borrowed values remain in use. */
char *nvram_get(const char *key);
int nvram_set(const char *key,const char *value);
int nvram_unset(const char *key);
int nvram_getall(char *output,int capacity);
char *wlcsm_nvram_get(const char *key);
int wlcsm_nvram_set(const char *key,const char *value);
int wlcsm_nvram_unset(const char *key);
int wlcsm_nvram_getall(char *output,int capacity);
/* Zero on saved-and-acknowledged success, one on failure. A failure may follow
 * successful persistence; errno carries the immediate error, not rollback. */
int wlcsm_nvram_commit(void);
int nvram_commit(void);
/* Returns borrowed kernel value, NULL when missing/error, (char *)-1 if denied. */
char *nvram_get_salt(void);
/* Direct kernel SET; NULL is encoded as a zero-length value, not UNSET. */
int nvram_kset(const char *key,const char *value);
int common_nvram_kernel_set(const char *key,const char *value);
/* Save kernel settings without a notification. Zero success, -1 failure;
 * errors can follow installation. Uses the source recovery-file transaction. */
int nvram_kcommit(void);
/* Shared borrowed conversion buffer; NULL on empty reply or error. */
char *wlcsm_nvram_xfr(const char *input);
char *nvram_xfr(const char *input);
/* Zero acknowledges a manager response, not proof that hardware restarted. */
int wlcsm_mngr_restart(unsigned radio,unsigned source,int persist,int wait);
char *wlcsm_nvram_get_bitflag(const char *key,unsigned bit);
int wlcsm_nvram_set_bitflag(const char *key,unsigned bit,int enabled);
#endif
