/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef OPEN_NVRAM_NOTIFICATION_H
#define OPEN_NVRAM_NOTIFICATION_H
#include "frame.h"
#include "pair.h"
#include <errno.h>
typedef void (*nv_invalidate_fn)(void *,const char *);
/* Caller must validate recvmsg source (kernel port 0, multicast group 1),
 * reject truncation and invalidate all on receive loss. Header pid identifies
 * the requesting process, not the sender, and must not authenticate a frame.
 * 1: key invalidated; 2: commit notice; 0: other operation; -1: malformed.
 * Malformed trusted traffic invalidates all. Callback key is borrowed only
 * for its duration. Broadcast values do not prove applied kernel state. */
static int nv_notification(const void *data,size_t size,nv_invalidate_fn invalidate,
                            void *context) {
    struct nv_frame_view frame;
    struct nv_pair_view pair;
    if(!invalidate) {errno=EINVAL;return -1;}
    if(nv_frame_decode(data,size,&frame) || frame.consumed!=size) goto malformed;
    if(frame.type==7) {
        if(frame.length) goto malformed;
        return 2;
    }
    if(frame.type!=2 && frame.type!=4) return 0;
    if(frame.length>=1024 || nv_pair_decode(frame.payload,frame.length,&pair) ||
       pair.consumed!=frame.length) goto malformed;
    /* No value copy: even an UNSET request carrying value bytes is handled by
     * the kernel using just its key. Refresh resolves actual applied state. */
    invalidate(context,pair.name);return 1;
malformed:
    invalidate(context,NULL);errno=EPROTO;return -1;
}
#endif
