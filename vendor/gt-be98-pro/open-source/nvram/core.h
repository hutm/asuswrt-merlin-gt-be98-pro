/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef OPEN_NVRAM_CORE_H
#define OPEN_NVRAM_CORE_H
#include <stddef.h>
#include <stdio.h>
struct nv_core;
typedef int (*nv_core_exchange)(void *,const void *,size_t,void *,size_t,size_t *);
/* Exchange/context must remain valid until destroy, and must not reenter this
 * core. Destroy requires all users to have stopped. All other calls serialize.
 * These are internal APIs, not the exported vendor library ABI. */
struct nv_core *nv_core_create(nv_core_exchange exchange,void *context,size_t cache_limit);
void nv_core_destroy(struct nv_core *core);
/* Fresh GET every time. Return 1 with caller-owned allocation, 0 with NULL for
 * missing, -1 with output unchanged. Caller frees successful non-NULL values. */
int nv_core_get_copy(struct nv_core *core,const char *key,char **value);
/* Compatibility-only borrowed view. Storage lasts until destroy, but a later
 * refresh of this key may overwrite it. Caller must serialize retained reads
 * with operations that refresh the same key. Prefer get_copy for new callers. */
int nv_core_get_borrowed(struct nv_core *core,const char *key,char **value);
/* Echo acknowledgement only, not applied-state or persistence confirmation. */
int nv_core_change_ack(struct nv_core *core,const char *key,const char *value,int unset);
int nv_core_getall(struct nv_core *core,void *output,size_t capacity,size_t *used);
/* Only call for trusted kernel notifications. data==NULL invalidates all after
 * loss/reopen. Otherwise returns notification decoder status. */
int nv_core_notification(struct nv_core *core,const void *data,size_t size);
/* Incremental load: each SET acknowledgement is followed by GET comparison.
 * confirmed counts matching readbacks, not a transaction or durable commits.
 * On failure, earlier entries and possibly the failing entry may have changed. */
int nv_core_load_verified(struct nv_core *core,FILE *input,size_t *confirmed);
/* Save current GETALL to the existing settings filenames in a trusted directory.
 * External cross-process writer serialization is required. Return 0 installed
 * and synced, -1 before installation failure, -2 installed but durability
 * uncertain. Empty replies are refused: kernel allocation failure looks empty.
 * This is not nvram_commit: ASUS controls and commit notification are separate. */
int nv_core_save_at(struct nv_core *core,int directory,size_t capacity);
/* Save then notify under the core mutex; external writer serialization required.
 * Save statuses apply, plus -3: installed and synced, notification unconfirmed.
 * Notification may have arrived on failure; no rollback. Zero confirms only an
 * echo, not subscriber receipt. ASUS suppression/markers belong in the wrapper. */
int nv_core_commit_at(struct nv_core *core,int directory,size_t capacity);
#endif
