/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef OPEN_NVRAM_PAIR_H
#define OPEN_NVRAM_PAIR_H
#include <limits.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
/* Native-endian 32-bit lengths are part of the kernel ABI. Buffers may be
 * unaligned. Encoder input/output must not overlap. Values may be binary. */
struct nv_pair_view {
    const char *name;
    const unsigned char *value;
    size_t name_len, value_len, consumed;
};
static int nv_pair_encode(void *output, size_t capacity, const char *name,
                          const void *value, size_t value_len, size_t *written)
{
    unsigned char *out = output;
    size_t name_len, aligned, total;
    uint32_t word;
    if (!output || !name || !written || (!value && value_len)) return -1;
    /* Bound the name scan by the maximum representable output. */
    for (name_len = 0; name_len < capacity && name[name_len]; ++name_len) {}
    if (name_len == capacity || name_len >= INT32_MAX) return -1;
    ++name_len;
    aligned = (name_len + 3) & ~(size_t)3;
    if (aligned > capacity || capacity - aligned < 8 ||
        value_len > capacity - aligned - 8 || value_len > INT32_MAX) return -1;
    total = aligned + 8 + value_len;
    memset(out, 0, total);
    word = (uint32_t)name_len; memcpy(out, &word, 4);
    memcpy(out + 4, name, name_len);
    word = (uint32_t)value_len; memcpy(out + 4 + aligned, &word, 4);
    if (value_len) memcpy(out + 8 + aligned, value, value_len);
    *written = total;
    return 0;
}
static int nv_pair_decode(const void *input, size_t size, struct nv_pair_view *view)
{
    const unsigned char *in = input;
    uint32_t name_len, value_len;
    size_t aligned;
    struct nv_pair_view result;
    if (!input || !view || size < 8) return -1;
    memcpy(&name_len, in, 4);
    if (!name_len || name_len > INT32_MAX || name_len > size - 4) return -1;
    if (in[4 + name_len - 1] || memchr(in + 4, 0, name_len - 1)) return -1;
    aligned = ((size_t)name_len + 3) & ~(size_t)3;
    if (aligned > size - 8) return -1;
    memcpy(&value_len, in + 4 + aligned, 4);
    if (value_len > INT32_MAX || value_len > size - aligned - 8) return -1;
    result.name = (const char *)in + 4;
    result.name_len = name_len - 1;
    result.value = value_len ? in + 8 + aligned : NULL;
    result.value_len = value_len;
    result.consumed = aligned + 8 + value_len;
    *view = result;
    return 0;
}
#endif
