/* SPDX-License-Identifier: GPL-2.0-only */
#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include "ring.h"

int main(void)
{
    struct ogso_ring q;
    struct ogso_job storage[32], batch[16], job;
    int packets[33], device, first, second;
    unsigned cycle, i, j;
    assert(ogso_ring_init(&q, storage, 0) == -1);
    assert(ogso_ring_init(&q, storage, 3) == -1);
    assert(ogso_ring_init(&q, storage, 32768) == -1);
    assert(ogso_ring_init(NULL, storage, 32) == -1);
    assert(ogso_ring_init(&q, NULL, 32) == -1);
    assert(ogso_ring_init(&q, storage, 32) == 0);
    job.device = &device;
    job.callback_context[0] = &first;
    job.callback_context[1] = &second;
    /* Repeat across multiple 16-bit counter wraps with full and empty boundaries. */
    for (cycle = 0; cycle < 8193; ++cycle) {
        for (i = 0; i < 32; ++i) {
            job.packet = &packets[i];
            assert(ogso_ring_push(&q, &job) == 1);
        }
        job.packet = &packets[32];
        assert(ogso_ring_push(&q, &job) == 0);
        assert(ogso_ring_count(&q) == 32);
        for (j = 0; j < 2; ++j) {
            assert(ogso_ring_batch(&q, batch) == 16);
            for (i = 0; i < 16; ++i) {
                assert(batch[i].packet == &packets[j * 16 + i]);
                assert(batch[i].device == &device);
                assert(batch[i].callback_context[0] == &first);
                assert(batch[i].callback_context[1] == &second);
            }
        }
        assert(ogso_ring_count(&q) == 0);
        assert(ogso_ring_batch(&q, batch) == 0);
    }
    job.packet = &packets[0];
    assert(ogso_ring_push(&q, &job) == 1);
    assert(ogso_ring_batch(&q, batch) == 1);
    assert(batch[0].packet == &packets[0]);
    puts("ring: capacity, FIFO, batch bounds, descriptors and counter wrap passed");
    return 0;
}
