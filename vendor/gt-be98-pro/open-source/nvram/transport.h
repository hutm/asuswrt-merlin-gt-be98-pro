/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef OPEN_NVRAM_TRANSPORT_H
#define OPEN_NVRAM_TRANSPORT_H
#include <errno.h>
#include <limits.h>
#include <poll.h>
#include <stdint.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/random.h>
#include <linux/netlink.h>
#include <time.h>
#include <unistd.h>
struct nv_transport {
    int protocol, timeout_ms;
    uint32_t peer;
    /* Only for the audited WLCSM kernel path: its skb portid is recipient ID. */
    int kernel_echoes_recipient;
};
static int64_t nv_now_ms(void) {
    struct timespec t;
    if(clock_gettime(CLOCK_MONOTONIC,&t)) return -1;
    return (int64_t)t.tv_sec*1000+t.tv_nsec/1000000;
}
static int nv_wait(int fd,short events,int64_t deadline) {
    struct pollfd p={fd,events,0};
    for(;;) {
        int64_t now=nv_now_ms(),remaining;
        int result;
        if(now<0) return -1;
        remaining=deadline-now;
        if(remaining<=0) { errno=ETIMEDOUT;return -1; }
        result=poll(&p,1,remaining>INT_MAX?INT_MAX:(int)remaining);
        if(result<0 && errno==EINTR) continue;
        if(result<=0) { if(!result) errno=ETIMEDOUT;return -1; }
        if(p.revents&events) return 0;
        errno=EIO;return -1;
    }
}
/* GET/SET request exchanges only. A separate persistent listener is required
 * for notifications. Random port IDs reduce delayed-reply reuse risk; the
 * protocol has no reliable sequence identifier, so this is not a proof of
 * correlation across arbitrarily many exchanges. */
static int nv_socket_exchange(void *context,const void *request,size_t length,
                              void *reply,size_t capacity,size_t *received)
{
    struct nv_transport *t=context;
    struct sockaddr_nl local={0},peer={0},sender;
    socklen_t local_size=sizeof local;
    unsigned char message[1280];
    int fd=-1,result=-1,saved;
    int64_t deadline;
    uint32_t expected;
    if(!t || !request || !reply || !received || length<24 || length>sizeof message ||
       !capacity || t->timeout_ms<=0 || (t->kernel_echoes_recipient && t->peer)) {
        errno=EINVAL;return -1;
    }
    deadline=nv_now_ms();if(deadline<0) return -1;
    deadline+=t->timeout_ms;
    fd=socket(AF_NETLINK,SOCK_RAW|SOCK_CLOEXEC|SOCK_NONBLOCK,t->protocol);
    if(fd<0) return -1;
    local.nl_family=AF_NETLINK;
    for(int attempt=0;;++attempt) {
        ssize_t count=getrandom(&local.nl_pid,sizeof local.nl_pid,GRND_NONBLOCK);
        if(count!=(ssize_t)sizeof local.nl_pid) { if(count>=0) errno=EIO;goto done; }
        local.nl_pid|=UINT32_C(0x80000000);
        if(!bind(fd,(struct sockaddr *)&local,sizeof local)) break;
        if(errno!=EADDRINUSE || attempt>=15) goto done;
    }
    if(getsockname(fd,(struct sockaddr *)&local,&local_size)) goto done;
    if(local_size!=sizeof local || !local.nl_pid) { errno=EPROTO;goto done; }
    expected=t->kernel_echoes_recipient?local.nl_pid:t->peer;
    peer.nl_family=AF_NETLINK;peer.nl_pid=t->peer;
    memcpy(message,request,length);memcpy(message+12,&local.nl_pid,4);
    for(;;) {
        ssize_t sent=sendto(fd,message,length,0,(struct sockaddr *)&peer,sizeof peer);
        if(sent==(ssize_t)length) break;
        if(sent<0 && errno==EINTR) { if(nv_wait(fd,POLLOUT,deadline)) goto done;continue; }
        if(sent<0 && (errno==EAGAIN || errno==EWOULDBLOCK)) {
            if(nv_wait(fd,POLLOUT,deadline)) goto done;
            continue;
        }
        if(sent>=0) errno=EIO;
        goto done;
    }
    for(;;) {
        struct iovec iov={reply,capacity};
        struct msghdr msg={0};
        ssize_t count;
        if(nv_wait(fd,POLLIN,deadline)) goto done;
        memset(&sender,0,sizeof sender);
        msg.msg_name=&sender;msg.msg_namelen=sizeof sender;
        msg.msg_iov=&iov;msg.msg_iovlen=1;
        count=recvmsg(fd,&msg,0);
        if(count<0 && (errno==EINTR || errno==EAGAIN || errno==EWOULDBLOCK)) continue;
        if(count<0) goto done;
        if(msg.msg_namelen!=sizeof sender || sender.nl_family!=AF_NETLINK ||
           sender.nl_pid!=expected || sender.nl_groups) continue;
        if(msg.msg_flags&MSG_TRUNC) { errno=EMSGSIZE;goto done; }
        *received=(size_t)count;result=0;break;
    }
done:
    saved=errno;close(fd);errno=saved;return result;
}
#endif
