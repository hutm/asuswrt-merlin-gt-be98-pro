/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef OPEN_NVRAM_STORAGE_LOCK_H
#define OPEN_NVRAM_STORAGE_LOCK_H
/* Existing firmware uses exclusive creation of /var/nvram.lock. Supply a
 * trusted open /var directory. Success returns an owned FD; failure is -1.
 * Each successful acquire must have exactly one release. The caller must
 * serialize threads and exclude fork/teardown while a lock is held. */
int nv_storage_lock_acquire(int directory,unsigned attempts,unsigned delay_ms);
int nv_storage_lock_release(int directory,int owned_fd);
#endif
