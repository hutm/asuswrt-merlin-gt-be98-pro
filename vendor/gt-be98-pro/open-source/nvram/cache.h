/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef OPEN_NVRAM_CACHE_H
#define OPEN_NVRAM_CACHE_H
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Internal storage, not yet the nvram_get ABI. All access, including reads via
 * retained pointers, requires external serialization. No eviction: addresses
 * remain valid until destruction, which requires all readers to have stopped.
 * Reserve full value capacity so growth never relocates an exposed buffer. */
struct nv_cache_entry {
    struct nv_cache_entry *next;
    int present;
    int stale;
    char value[1024];
    char key[];
};
struct nv_cache {
    struct nv_cache_entry *buckets[32];
    size_t used,limit;
    uint64_t generation;
};
static unsigned nv_cache_hash(const char *key) {
    uint32_t hash=0;
    while(*key) hash=hash*31+(unsigned char)*key++;
    return hash%32;
}
static struct nv_cache_entry *nv_cache_find(struct nv_cache *cache,const char *key) {
    struct nv_cache_entry *entry;
    for(entry=cache->buckets[nv_cache_hash(key)];entry;entry=entry->next)
        if(!strcmp(entry->key,key)) return entry;
    return NULL;
}
/* 1 = present (including empty), 0 = absent/unknown, -1 = invalid input or stale.
 * On error, preserve output. Missing and never fetched differ at the client
 * layer; this storage does not claim freshness or perform negative caching. */
static int nv_cache_lookup(struct nv_cache *cache,const char *key,char **value) {
    struct nv_cache_entry *entry;
    if(!cache || !key || !value) {errno=EINVAL;return -1;}
    entry=nv_cache_find(cache,key);
    if(entry && entry->stale) {errno=EAGAIN;return -1;}
    *value=entry && entry->present?entry->value:NULL;
    return *value?1:0;
}
/* A successful missing update clears a retained buffer to an empty string.
 * Failures leave existing data and output untouched. Never pass a failed GET
 * as a missing update. The caller decides whether stale data is usable. */
static int nv_cache_store(struct nv_cache *cache,const char *key,
                          const char *value,char **retained) {
    size_t key_size=0,value_size=0,total;
    struct nv_cache_entry *entry;
    unsigned bucket;
    if(!cache || !key || !retained) {errno=EINVAL;return -1;}
    while(key_size<1024 && key[key_size]) ++key_size;
    if(key_size==1024) {errno=EOVERFLOW;return -1;}
    if(value) {
        while(value_size<1024 && value[value_size]) ++value_size;
        if(value_size==1024) {errno=EOVERFLOW;return -1;}
        ++value_size;
    }
    entry=nv_cache_find(cache,key);
    if(!entry && !value) {*retained=NULL;return 0;}
    if(!entry) {
        total=sizeof *entry+key_size+1;
        if(cache->used>cache->limit || total>cache->limit-cache->used) {
            errno=ENOSPC;return -1;
        }
        entry=calloc(1,total);if(!entry) return -1;
        memcpy(entry->key,key,key_size+1);
        bucket=nv_cache_hash(key);
        entry->next=cache->buckets[bucket];cache->buckets[bucket]=entry;
        cache->used+=total;
    }
    /* memmove allows updating from a retained value or a suffix of it. */
    if(value) memmove(entry->value,value,value_size);
    else entry->value[0]=0;
    entry->present=value!=NULL;
    entry->stale=0;
    *retained=entry->present?entry->value:NULL;
    return entry->present?1:0;
}
/* Fetch owns a newly allocated value on return 1, returns NULL on 0, and
 * preserves its output on -1. Its caller can adapt nv_client_get here. */
typedef int (*nv_cache_fetch)(void *,const char *,char **);
/* Invalidation never overwrites values held by existing readers. Callers must
 * refresh before using a stale entry for a new lookup. Unknown-key events still
 * advance generation to catch a notification during its first fetch. */
static void nv_cache_invalidate(struct nv_cache *cache,const char *key) {
    struct nv_cache_entry *entry;
    if(!cache) return;
    ++cache->generation;
    if(key) {
        entry=nv_cache_find(cache,key);
        if(entry) entry->stale=1;
    } else {
        for(unsigned i=0;i<32;++i)
            for(entry=cache->buckets[i];entry;entry=entry->next) entry->stale=1;
    }
}
static int nv_cache_refresh(struct nv_cache *cache,nv_cache_fetch fetch,void *context,
                            const char *key,char **retained) {
    char *value=NULL;
    int status,result,saved;
    uint64_t generation;
    if(!cache || !fetch || !key || !retained) {errno=EINVAL;return -1;}
    generation=cache->generation;
    status=fetch(context,key,&value);
    if(status<0) {saved=errno;free(value);errno=saved;return -1;}
    if((status!=0 && status!=1) || (status==1 && !value) || (status==0 && value)) {
        free(value);errno=EPROTO;return -1;
    }
    if(generation!=cache->generation) {free(value);errno=EAGAIN;return -1;}
    result=nv_cache_store(cache,key,value,retained);
    saved=errno;free(value);errno=saved;return result;
}
static void nv_cache_destroy(struct nv_cache *cache) {
    if(!cache) return;
    for(unsigned i=0;i<32;++i) {
        struct nv_cache_entry *entry=cache->buckets[i];
        while(entry) {
            struct nv_cache_entry *next=entry->next;
            free(entry);entry=next;
        }
        cache->buckets[i]=NULL;
    }
    cache->used=0;
}
#endif
