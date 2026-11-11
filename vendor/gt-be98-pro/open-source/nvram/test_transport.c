#define _POSIX_C_SOURCE 200809L
#include "transport.h"
#include "getall.h"
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
struct server { int fd,mode; };
static void *serve(void *opaque) {
    struct server *s=opaque;
    unsigned char request[1280],reply[1280];
    struct sockaddr_nl client;
    socklen_t size=sizeof client;
    size_t length;
    ssize_t n=recvfrom(s->fd,request,sizeof request,0,(struct sockaddr *)&client,&size);
    assert(n>0);
    if(s->mode==1) return NULL;
    if(s->mode==3) {
        assert(n>1024);
        assert(sendto(s->fd,request,(size_t)n,0,(struct sockaddr *)&client,size)==n);
        return NULL;
    }
    assert(!nv_frame_encode(reply,sizeof reply,6,0,0,"key=value",10,&length));
    /* The header can lie: origin must come from recvmsg's sockaddr. */
    if(s->mode==2) {
        int bad=socket(AF_NETLINK,SOCK_RAW,NETLINK_USERSOCK);assert(bad>=0);
        assert(sendto(bad,reply,length,0,(struct sockaddr *)&client,size)==(ssize_t)length);
        close(bad);return NULL;
    }
    assert(sendto(s->fd,reply,length,0,(struct sockaddr *)&client,size)==(ssize_t)length);
    return NULL;
}
int main(void) {
    struct sockaddr_nl address={0};
    socklen_t address_size=sizeof address;
    struct server server;
    struct nv_transport transport={NETLINK_USERSOCK,200,0,0};
    pthread_t thread;
    unsigned char out[64];size_t used;
    server.fd=socket(AF_NETLINK,SOCK_RAW,NETLINK_USERSOCK);assert(server.fd>=0);
    address.nl_family=AF_NETLINK;
    assert(!bind(server.fd,(struct sockaddr *)&address,sizeof address));
    assert(!getsockname(server.fd,(struct sockaddr *)&address,&address_size));
    transport.peer=address.nl_pid;
    for(server.mode=0;server.mode<3;++server.mode) {
        memset(out,0x55,sizeof out);used=999;
        assert(!pthread_create(&thread,NULL,serve,&server));
        int result=nv_getall_pages(nv_socket_exchange,&transport,0,out,sizeof out,&used);
        assert(!pthread_join(thread,NULL));
        if(!server.mode) assert(!result && used==10 && !strcmp((char *)out,"key=value"));
        else {
            assert(result==-1 && errno==ETIMEDOUT && used==999);
            for(size_t i=0;i<sizeof out;++i) assert(out[i]==0x55);
        }
    }
    {
        unsigned char payload[1023],request[1280],reply[1280];
        size_t length,received=0;struct nv_frame_view view;
        memset(payload,0x42,sizeof payload);server.mode=3;
        assert(!nv_frame_encode(request,sizeof request,2,0,0,payload,sizeof payload,&length));
        assert(!pthread_create(&thread,NULL,serve,&server));
        assert(!nv_socket_exchange(&transport,request,length,reply,sizeof reply,&received));
        assert(!pthread_join(thread,NULL));
        assert(!nv_frame_decode(reply,received,&view) && view.length==sizeof payload);
        assert(!memcmp(view.payload,payload,sizeof payload));
    }
    close(server.fd);
    puts("Netlink transport: real usersock exchange, maximum payload, deadline and forged-header wrong-sender rejection passed");
    return 0;
}
