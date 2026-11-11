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
#endif
