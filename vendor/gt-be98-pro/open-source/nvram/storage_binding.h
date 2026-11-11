/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef OPEN_NVRAM_STORAGE_BINDING_H
#define OPEN_NVRAM_STORAGE_BINDING_H
#include "adapter.h"
#include <pthread.h>
struct nv_storage_binding {int directory,owned_fd;pthread_mutex_t mutex;};
/* Supply a trusted /var directory; init retains its own descriptor. Hooks call
 * the real shared-library storage/policy exports. Destroy requires quiescence. */
int nv_storage_binding_init(struct nv_storage_binding *,int,struct nv_storage_hooks *);
int nv_storage_binding_destroy(struct nv_storage_binding *);
#endif
