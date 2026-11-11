/* SPDX-License-Identifier: GPL-2.0-only */
/* Source replacement queue core. Caller must serialize all accesses.
 * Not a kernel module: worker lifecycle and packet ownership live above this API.
 */
#ifndef OPEN_GSO_RING_H
#define OPEN_GSO_RING_H
#ifdef __KERNEL__
#include <linux/types.h>
typedef u16 ogso_u16;
#else
#include <stdint.h>
typedef uint16_t ogso_u16;
#endif

struct ogso_job {
    void *device;
    void *packet;
    /* Callback types must be established before a kernel integration. */
    void *callback_context[2];
};

struct ogso_ring {
    struct ogso_job *jobs;
    ogso_u16 producer;
    ogso_u16 consumer;
    ogso_u16 capacity;
};

static inline int ogso_ring_init(struct ogso_ring *q,
                                struct ogso_job *storage, unsigned capacity)
{
    if (!q || !storage || !capacity || capacity >= 32768 ||
        (capacity & (capacity - 1)))
        return -1;
    q->jobs = storage;
    q->producer = q->consumer = 0;
    q->capacity = (ogso_u16)capacity;
    return 0;
}

static inline unsigned ogso_ring_count(const struct ogso_ring *q)
{
    return (ogso_u16)(q->producer - q->consumer);
}

/* Success transfers the descriptor only; this layer never frees a packet. */
static inline int ogso_ring_push(struct ogso_ring *q, const struct ogso_job *job)
{
    if (ogso_ring_count(q) >= q->capacity)
        return 0;
    q->jobs[q->producer & (q->capacity - 1)] = *job;
    q->producer = (ogso_u16)(q->producer + 1);
    return 1;
}

/* Copy up to 16 jobs, matching the observed worker batch bound. Caller holds
 * the queue lock while copying, then releases it before invoking callbacks.
 */
static inline unsigned ogso_ring_batch(struct ogso_ring *q,
                                      struct ogso_job out[16])
{
    unsigned count = 0;
    while (count < 16 && q->consumer != q->producer) {
        out[count++] = q->jobs[q->consumer & (q->capacity - 1)];
        q->consumer = (ogso_u16)(q->consumer + 1);
    }
    return count;
}
#endif
