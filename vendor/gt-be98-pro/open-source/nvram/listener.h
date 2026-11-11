/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef OPEN_NVRAM_LISTENER_H
#define OPEN_NVRAM_LISTENER_H
#include "notification.h"
#include <linux/netlink.h>
#include <sys/socket.h>
#include <unistd.h>
/* Caller owns this persistent nonblocking socket and arranges polling, shutdown
 * and cache locking. Subscription must precede initial cache population. */
static int nv_listener_open(int protocol) {
    struct sockaddr_nl address={0};int fd,saved;
    fd=socket(AF_NETLINK,SOCK_RAW|SOCK_NONBLOCK|SOCK_CLOEXEC,protocol);
    if(fd<0) return -1;
    address.nl_family=AF_NETLINK;address.nl_groups=1;
    if(bind(fd,(struct sockaddr *)&address,sizeof address)) {
        saved=errno;close(fd);errno=saved;return -1;
    }
    return fd;
}
/* Receive one datagram. Same return values as nv_notification; EAGAIN means
 * drained, EINTR means retry. Any other receive failure invalidates all because
 * continuity cannot be assumed. Untrusted traffic is discarded without using
 * header pid as proof of kernel origin. */
static int nv_listener_receive(int fd,nv_invalidate_fn invalidate,void *context) {
    unsigned char bytes[1280];struct sockaddr_nl sender={0};
    struct iovec iov={bytes,sizeof bytes};struct msghdr message={0};
    ssize_t received;int saved;
    if(fd<0 || !invalidate) {errno=EINVAL;return -1;}
    message.msg_name=&sender;message.msg_namelen=sizeof sender;
    message.msg_iov=&iov;message.msg_iovlen=1;
    received=recvmsg(fd,&message,MSG_DONTWAIT);
    if(received<0) {
        saved=errno;
        if(saved!=EAGAIN && saved!=EWOULDBLOCK && saved!=EINTR)
            invalidate(context,NULL);
        errno=saved;return -1;
    }
    if(message.msg_namelen!=sizeof sender || sender.nl_family!=AF_NETLINK ||
       sender.nl_pid!=0 || sender.nl_groups!=1) return 0;
    if(message.msg_flags&MSG_TRUNC || (size_t)received>sizeof bytes) {
        invalidate(context,NULL);errno=EMSGSIZE;return -1;
    }
    return nv_notification(bytes,(size_t)received,invalidate,context);
}
#endif
