/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef OPEN_NVRAM_FRAME_H
#define OPEN_NVRAM_FRAME_H
#include <stdint.h>
#include <stddef.h>
#include <string.h>
struct nv_frame_view {
    uint16_t type;
    const unsigned char *payload;
    size_t length, consumed;
};
/* Fixed Linux Netlink header (16 bytes) plus WLCSM header (8 bytes).
 * The existing kernel does not initialize nlmsg_type/seq or WLCSM pid in
 * replies. Do not use those fields for matching or authentication. Transport
 * origin, expected operation and socket serialization require separate checks. */
static int nv_frame_encode(void *output, size_t capacity, uint16_t type,
                           uint32_t sender, uint32_t destination,
                           const void *payload, size_t length, size_t *written)
{
    unsigned char *out=output;
    size_t total;
    uint32_t word;
    uint16_t shortword;
    if(!output || !written || (!payload && length) || length>UINT16_MAX) return -1;
    total=(24+length+3)&~(size_t)3;
    if(capacity<total) return -1;
    memset(out,0,total);
    word=(uint32_t)total;memcpy(out,&word,4);
    memcpy(out+12,&sender,4);
    memcpy(out+16,&type,2);
    shortword=(uint16_t)length;memcpy(out+18,&shortword,2);
    memcpy(out+20,&destination,4);
    if(length) memcpy(out+24,payload,length);
    *written=total;
    return 0;
}
static int nv_frame_decode(const void *input, size_t available, struct nv_frame_view *view)
{
    const unsigned char *in=input;
    uint32_t total;
    uint16_t type,length;
    struct nv_frame_view result;
    if(!input || !view || available<24) return -1;
    memcpy(&total,in,4);
    if(total<24 || total>available) return -1;
    memcpy(&type,in+16,2);memcpy(&length,in+18,2);
    if((size_t)length>total-24 || total-24-(size_t)length>3) return -1;
    result.type=type;result.length=length;
    result.payload=in+24;result.consumed=total;
    *view=result;
    return 0;
}
#endif
