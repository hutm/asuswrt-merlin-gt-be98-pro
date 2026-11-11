#include "frame.h"
#include <linux/netlink.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
struct reference_header { unsigned short type,len; unsigned int pid; };
int main(void) {
    union { uint64_t alignment; unsigned char bytes[2048]; } reference;
    unsigned char encoded[2049],payload[1024];
    struct nv_frame_view view;
    size_t n,written,cut;
    uint32_t invalid;
    _Static_assert(sizeof(struct nlmsghdr)==16,"Netlink ABI");
    _Static_assert(sizeof(struct reference_header)==8,"WLCSM ABI");
    for(n=0;n<sizeof payload;++n) payload[n]=(unsigned char)n;
    for(n=0;n<=sizeof payload;++n) {
        /* Reproduce the assignments in kernel wlcsm_sendup_response. Leave
         * unspecified fields nonzero to ensure parser does not rely on them. */
        struct nlmsghdr *nlh=(void *)reference.bytes;
        struct reference_header *header=NLMSG_DATA(nlh);
        memset(reference.bytes,0xa5,sizeof reference.bytes);
        nlh->nlmsg_len=NLMSG_SPACE(sizeof *header+n);
        nlh->nlmsg_pid=0;nlh->nlmsg_flags=0;
        header->type=3;header->len=(unsigned short)n;
        memcpy(header+1,payload,n);
        assert(nv_frame_decode(reference.bytes,nlh->nlmsg_len,&view)==0);
        assert(view.type==3 && view.length==n && !memcmp(view.payload,payload,n));
        assert(nv_frame_encode(encoded+1,2048,3,0,0,payload,n,&written)==0);
        assert(written==nlh->nlmsg_len);
        assert(nv_frame_decode(encoded+1,written,&view)==0);
        assert(view.type==3 && view.length==n && !memcmp(view.payload,payload,n));
        for(cut=0;cut<written;++cut) assert(nv_frame_decode(encoded+1,cut,&view)==-1);
        assert(nv_frame_encode(encoded+1,written-1,3,0,0,payload,n,&cut)==-1);
    }
    assert(nv_frame_encode(encoded,sizeof encoded,6,0,0,NULL,0,&written)==0);
    assert(nv_frame_decode(encoded,written,&view)==0 && !view.length);
    invalid=UINT32_MAX;memcpy(encoded,&invalid,4);
    assert(nv_frame_decode(encoded,sizeof encoded,&view)==-1);
    assert(nv_frame_encode(encoded,sizeof encoded,3,0,0,NULL,1,&written)==-1);
    assert(nv_frame_encode(encoded,sizeof encoded,3,0,0,payload,65536,&written)==-1);
    puts("NVRAM frames: kernel reply layout, undefined header fields, truncation and bounds passed");
    return 0;
}
