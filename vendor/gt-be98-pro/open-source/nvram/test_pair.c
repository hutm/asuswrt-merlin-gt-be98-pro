#include "pair.h"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#include "reference.h"
#pragma GCC diagnostic pop
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
int main(void) {
    union { uint32_t alignment; unsigned char bytes[2048]; } reference;
    unsigned char candidate[2049], value[512];
    char name[257];
    struct nv_pair_view view;
    size_t written, n, v, cut;
    uint32_t random=0x98;
    for(n=0;n<sizeof value;++n) value[n]=(unsigned char)n;
    for(n=0;n<256;++n) {
        memset(name,'a',n);name[n]=0;
        for(v=0;v<sizeof value;v+=17) {
            _valuepair_set((void *)reference.bytes,name,v?(char *)value:NULL,(int)v);
            assert(nv_pair_encode(candidate+1,2048,name,v?value:NULL,v,&written)==0);
            assert(written==(size_t)_get_valuepair_total_len(name,v?(char *)value:NULL,(int)v));
            assert(!memcmp(reference.bytes,candidate+1,written));
            assert(nv_pair_decode(candidate+1,written,&view)==0);
            assert(view.name_len==n && !strcmp(view.name,name));
            assert(view.value_len==v && view.consumed==written);
            assert(v ? !memcmp(view.value,value,v) : view.value==NULL);
            for(cut=0;cut<written;++cut)
                assert(nv_pair_decode(candidate+1,cut,&view)==-1);
            assert(nv_pair_encode(candidate+1,written-1,name,v?value:NULL,v,&cut)==-1);
        }
    }
    assert(nv_pair_encode(candidate,sizeof candidate,"key","",1,&written)==0);
    assert(nv_pair_decode(candidate,written,&view)==0 && view.value && view.value_len==1 && !*view.value);
    assert(nv_pair_encode(candidate,sizeof candidate,"key",NULL,1,&written)==-1);
    /* Arbitrary malformed buffers: any successful decode must stay in bounds. */
    for(n=0;n<10000;++n) {
        size_t length=n%sizeof candidate;
        for(v=0;v<length;++v) { random=random*1664525+1013904223;candidate[v]=(unsigned char)(random>>24); }
        if(nv_pair_decode(candidate,length,&view)==0) {
            assert(view.consumed<=length);
            assert(strlen(view.name)==view.name_len);
            assert(!view.value || view.value+view.value_len<=candidate+length);
        }
    }
    puts("NVRAM pair: kernel-source byte comparison, truncation, unaligned buffers and malformed input passed");
    return 0;
}
