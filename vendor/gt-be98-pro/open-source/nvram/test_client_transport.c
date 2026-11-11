/* SPDX-License-Identifier: GPL-2.0-only */
#define _POSIX_C_SOURCE 200809L
#include "client.h"
#include "transport.h"
#include <assert.h>
#include <pthread.h>
#include <stdio.h>

/* A userspace protocol peer, not a substitute for a router kernel test. */
struct server { int fd, present, ignore_next; char value[1011]; };
static void *serve(void *opaque) {
    struct server *s=opaque;
    unsigned char request[1280],reply[1280],payload[1024];
    struct sockaddr_nl client={0}; socklen_t address_size=sizeof client;
    struct nv_frame_view frame; struct nv_pair_view pair;
    size_t length=0,reply_size; uint32_t sender;
    ssize_t count=recvfrom(s->fd,request,sizeof request,0,
                           (struct sockaddr *)&client,&address_size);
    assert(count>=24 && !nv_frame_decode(request,(size_t)count,&frame));
    memcpy(&sender,request+12,4);
    assert(sender==client.nl_pid && sender>=UINT32_C(0x80000000));
    if(frame.type==3) {
        assert(frame.length==4 && !memcmp(frame.payload,"key",4));
        if(s->present)
            assert(!nv_pair_encode(payload,sizeof payload,"key",s->value,
                                   strlen(s->value)+1,&length));
    } else {
        assert(frame.type==2 || frame.type==4);
        assert(!nv_pair_decode(frame.payload,frame.length,&pair));
        assert(!strcmp(pair.name,"key") && pair.value_len<=sizeof s->value);
        if(!s->ignore_next) {
            s->present=frame.type==2 && pair.value_len;
            if(s->present) memcpy(s->value,pair.value,pair.value_len);
        }
        s->ignore_next=0;
        length=frame.length;memcpy(payload,frame.payload,length);
    }
    assert(!nv_frame_encode(reply,sizeof reply,frame.type,0,0,payload,length,&reply_size));
    assert(sendto(s->fd,reply,reply_size,0,(struct sockaddr *)&client,address_size)==(ssize_t)reply_size);
    return NULL;
}
static void change(struct server *s,struct nv_transport *t,const char *value,int unset) {
    pthread_t thread;
    assert(!pthread_create(&thread,NULL,serve,s));
    assert(!nv_client_change_ack(nv_socket_exchange,t,0,"key",value,unset));
    assert(!pthread_join(thread,NULL));
}
static void check(struct server *s,struct nv_transport *t,const char *expected) {
    pthread_t thread;char *value=NULL;
    assert(!pthread_create(&thread,NULL,serve,s));
    int result=nv_client_get(nv_socket_exchange,t,0,"key",&value);
    assert(!pthread_join(thread,NULL));
    if(expected) assert(result==1 && value && !strcmp(value,expected));
    else assert(result==0 && !value);
    free(value);
}
int main(void) {
    struct server s={0}; struct sockaddr_nl address={0};
    socklen_t size=sizeof address;
    struct nv_transport t={NETLINK_USERSOCK,1000,0,0};
    char maximum[1011];
    s.fd=socket(AF_NETLINK,SOCK_RAW,NETLINK_USERSOCK);assert(s.fd>=0);
    address.nl_family=AF_NETLINK;
    assert(!bind(s.fd,(struct sockaddr *)&address,sizeof address));
    assert(!getsockname(s.fd,(struct sockaddr *)&address,&size));t.peer=address.nl_pid;
    check(&s,&t,NULL);
    change(&s,&t,"",0);check(&s,&t,"");
    change(&s,&t,"value",0);check(&s,&t,"value");
    s.ignore_next=1;change(&s,&t,"unapplied",0);check(&s,&t,"value");
    memset(maximum,'x',sizeof maximum-1);maximum[sizeof maximum-1]=0;
    change(&s,&t,maximum,0);check(&s,&t,maximum);
    change(&s,&t,NULL,1);check(&s,&t,NULL);
    close(s.fd);
    puts("NVRAM client/transport: real Netlink round trips, empty/missing, maximum SET/GET, UNSET, and unapplied ACK passed");
    return 0;
}
