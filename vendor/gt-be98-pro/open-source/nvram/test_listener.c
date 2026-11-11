/* SPDX-License-Identifier: GPL-2.0-only */
#define _GNU_SOURCE
#include <sys/socket.h>
#include <linux/netlink.h>
#include <errno.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
static unsigned char datagram[1280];static size_t length;
static int receive_error,source_pid,source_group=1,flags,short_address;
static int bind_error,closed;
static int fake_socket(int domain,int type,int protocol) {
    assert(domain==AF_NETLINK && (type&SOCK_NONBLOCK) && (type&SOCK_CLOEXEC));
    assert(protocol==31);return 42;
}
static int fake_bind(int fd,const struct sockaddr *address,socklen_t size) {
    const struct sockaddr_nl *nl=(const struct sockaddr_nl *)address;
    assert(fd==42 && size==sizeof *nl && nl->nl_family==AF_NETLINK && nl->nl_groups==1);
    if(bind_error) {errno=EPERM;return -1;}return 0;
}
static int fake_close(int fd) {assert(fd==42);++closed;return 0;}
static ssize_t fake_receive(int fd,struct msghdr *message,int options) {
    struct sockaddr_nl *sender=message->msg_name;
    assert(fd==42 && options==MSG_DONTWAIT && message->msg_iovlen==1);
    if(receive_error) {errno=receive_error;return -1;}
    assert(message->msg_iov[0].iov_len>=length);
    memcpy(message->msg_iov[0].iov_base,datagram,length);
    sender->nl_family=AF_NETLINK;sender->nl_pid=source_pid;sender->nl_groups=source_group;
    message->msg_namelen=short_address?1:sizeof *sender;message->msg_flags=flags;
    return length;
}
#define socket fake_socket
#define bind fake_bind
#define close fake_close
#define recvmsg fake_receive
#include "listener.h"
#undef socket
#undef bind
#undef close
#undef recvmsg
static int keys,all;
static void invalidate(void *context,const char *key) {
    (void)context;if(key){assert(!strcmp(key,"key"));++keys;}else ++all;
}
int main(void) {
    unsigned char pair[1024];size_t size;
    assert(nv_listener_open(31)==42 && closed==0);
    bind_error=1;assert(nv_listener_open(31)==-1 && errno==EPERM && closed==1);
    assert(!nv_pair_encode(pair,sizeof pair,"key","value",6,&size));
    assert(!nv_frame_encode(datagram,sizeof datagram,2,12345,0,pair,size,&length));
    assert(nv_listener_receive(42,invalidate,NULL)==1 && keys==1);
    source_pid=555;assert(nv_listener_receive(42,invalidate,NULL)==0 && keys==1 && !all);
    source_pid=0;source_group=0;
    assert(nv_listener_receive(42,invalidate,NULL)==0 && !all);
    source_group=1;short_address=1;
    assert(nv_listener_receive(42,invalidate,NULL)==0 && !all);
    short_address=0;flags=MSG_TRUNC;
    assert(nv_listener_receive(42,invalidate,NULL)==-1 && errno==EMSGSIZE && all==1);
    flags=0;receive_error=ENOBUFS;
    assert(nv_listener_receive(42,invalidate,NULL)==-1 && errno==ENOBUFS && all==2);
    receive_error=EAGAIN;
    assert(nv_listener_receive(42,invalidate,NULL)==-1 && errno==EAGAIN && all==2);
    receive_error=EINTR;
    assert(nv_listener_receive(42,invalidate,NULL)==-1 && errno==EINTR && all==2);
    puts("NVRAM listener: subscription cleanup, sender metadata, truncation, loss and transient receive errors passed (mock sockets)");
    return 0;
}
