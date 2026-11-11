/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef OPEN_NVRAM_ADAPTER_H
#define OPEN_NVRAM_ADAPTER_H
#include "core.h"
/* Dependencies remain alive for the adapter lifetime. Storage hooks wrap the
 * shared-library JFFS functions and supply cross-process serialization. All
 * callbacks are mandatory; missing integration fails rather than bypassing it. */
struct nv_storage_hooks {
    void *context;
    int (*is_large)(void *,const char *);
    int (*lock)(void *); /* zero success, nonzero failure */
    int (*unlock)(void *); /* zero success; release failure is reported */
    char *(*get)(void *,const char *);
    int (*set)(void *,const char *,const char *);
    int (*unset)(void *,const char *);
    int (*getall)(void *,int,char *,int);
    int (*deny_get)(void *,const char *);
    int (*deny_getall)(void *);
};
struct nv_adapter {struct nv_core *core;struct nv_storage_hooks storage;};
int nv_adapter_init(struct nv_adapter *,struct nv_core *,const struct nv_storage_hooks *);
/* Borrowed results follow the selected backend lifetime. No concurrent-read
 * guarantee is added for pointers used after the operation returns. */
char *nv_adapter_get(struct nv_adapter *,const char *);
int nv_adapter_set(struct nv_adapter *,const char *,const char *);
int nv_adapter_unset(struct nv_adapter *,const char *);
/* Returns used byte count, not zero-on-success. Success zero-fills unused space.
 * Errors preserve the caller buffer; enumeration is not an atomic snapshot. */
int nv_adapter_getall(struct nv_adapter *,char *,int);
#endif
