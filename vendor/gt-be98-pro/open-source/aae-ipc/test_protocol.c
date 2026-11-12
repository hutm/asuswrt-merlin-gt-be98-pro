/* SPDX-License-Identifier: GPL-2.0-only */
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>
extern int aae_sendIpcMsg(char *,char *,int);
extern int aae_sendIpcMsgAndWaitResp(char *,char *,int,char *,int,int);
static int descriptor,async_connect,response,length,reply_length,closes,sends,reads,waits;
static int fault,poll_calls,clock_calls,read_calls;
static size_t sent_total;
static int64_t fake_ms=100000;
static unsigned char data[2048],expected_packet[2060],reply[2060];
int __wrap_socket(int domain,int type,int protocol){assert(domain==AF_UNIX&&(type&~SOCK_CLOEXEC)==SOCK_STREAM&&!protocol);
#ifdef CANDIDATE_ONLY
 assert(type&SOCK_CLOEXEC);
#endif
if(fault==1){errno=EMFILE;return -1;}return descriptor;}
int __wrap_fcntl(int fd,int cmd,...){assert(fd==descriptor);if(cmd==F_GETFL){if(fault==2){errno=EIO;return -1;}return O_RDWR;}assert(cmd==F_SETFL);va_list ap;va_start(ap,cmd);int flags=va_arg(ap,int);va_end(ap);assert(flags==(O_RDWR|O_NONBLOCK));if(fault==3){errno=EIO;return -1;}return 0;}
int __wrap_fcntl64(int fd,int cmd,...){
 if(cmd==F_GETFL)return __wrap_fcntl(fd,cmd);
 va_list ap;va_start(ap,cmd);int flags=va_arg(ap,int);va_end(ap);
 return __wrap_fcntl(fd,cmd,flags);
}
int __wrap_connect(int fd,const struct sockaddr *addr,socklen_t n){assert(fd==descriptor&&n==sizeof(struct sockaddr_un));const struct sockaddr_un *a=(const void *)addr;assert(a->sun_family==AF_UNIX&&!strcmp(a->sun_path,"/tmp/test-ipc"));if(fault==4){errno=ECONNREFUSED;return -1;}if(async_connect||(fault>=5&&fault<=7)){errno=EINPROGRESS;return -1;}return 0;}
int __wrap_getsockopt(int fd,int level,int opt,void *value,socklen_t *n){assert(fd==descriptor&&level==SOL_SOCKET&&opt==SO_ERROR&&*n==sizeof(int));if(fault==8){errno=EIO;return -1;}*(int *)value=fault==9?ECONNREFUSED:0;return 0;}
ssize_t __wrap_send(int fd,const void *p,size_t n,int flags){
 assert(fd==descriptor&&n==2060-sent_total&&flags==MSG_NOSIGNAL);
 assert(!memcmp(p,expected_packet+sent_total,n));sends++;
 if(fault==10){errno=EPIPE;return -1;}if(fault==11)return 0;
 if((fault==13||fault==14)&&sends==1){errno=fault==13?EAGAIN:EINTR;return -1;}
 if(fault==12&&sends==1)n=17;
 sent_total+=n;return (ssize_t)n;
}
int __wrap_select(int n,fd_set *r,fd_set *w,fd_set *e,struct timeval *t){assert(n==descriptor+1&&!e&&t);assert((r!=NULL)!=(w!=NULL));assert(FD_ISSET(descriptor,r?r:w));if(w)assert(t->tv_sec==2&&!t->tv_usec);else assert(!t->tv_sec&&t->tv_usec==100000);waits++;return 1;}
int __wrap_poll(struct pollfd *p,nfds_t n,int timeout){
 assert(n==1&&p->fd==descriptor&&timeout>0);
 assert(p->events==POLLIN||p->events==POLLOUT);poll_calls++;waits++;
 if(fault==5||fault==15){fake_ms+=timeout;return 0;}
 if(fault==6||fault==16){errno=EIO;return -1;}
 if((fault==7||fault==17)&&poll_calls==1){fake_ms++;errno=EINTR;return -1;}
 p->revents=p->events;return 1;
}
int __wrap___poll_chk(struct pollfd *p,nfds_t n,int timeout,size_t bytes){assert(bytes>=n*sizeof(*p));return __wrap_poll(p,n,timeout);}
int __wrap_clock_gettime(clockid_t id,struct timespec *t){
 assert(id==CLOCK_MONOTONIC);clock_calls++;
 if(fault>=21&&fault<=23&&clock_calls==fault-20){errno=EIO;return -1;}
 t->tv_sec=fake_ms/1000;t->tv_nsec=(fake_ms%1000)*1000000;return 0;
}
ssize_t __wrap_read(int fd,void *p,size_t n){
 assert(fd==descriptor&&n==2060);read_calls++;
 if(fault==18){errno=EIO;return -1;}
 if((fault==19||fault==20)&&read_calls==1){errno=fault==19?EAGAIN:EINTR;return -1;}
 memcpy(p,reply,(size_t)reply_length);reads++;return reply_length;
}
ssize_t __wrap___read_chk(int fd,void *p,size_t n,size_t bytes){assert(bytes>=n);return __wrap_read(fd,p,n);}
int __wrap_close(int fd){assert(fd==descriptor);closes++;if(fault)errno=EBADF;return 0;}
int __wrap_printf(const char *format,...){(void)format;return 0;}
int __wrap_puts(const char *s){(void)s;return 0;}
static uint32_t hash(const unsigned char *p,size_t n){uint32_t v=2166136261u;for(size_t i=0;i<n;i++)v=(v^p[i])*16777619u;return v;}
int main(void){
 int lengths[]={0,1,15,127,1024,2047,2048},replies[]={0,1,17,2048,2060},capacities[]={1,2,16,2048,2060,2071},fds[]={0,5,31,32,1023};unsigned cases=0;
 for(unsigned i=0;i<sizeof(data);i++)data[i]=(unsigned char)(i*131+17);
 for(unsigned i=0;i<sizeof(reply);i++)reply[i]=(unsigned char)(i*137+19);
 for(response=0;response<2;response++)for(unsigned l=0;l<7;l++)for(unsigned r=0;r<5;r++)for(unsigned c=0;c<6;c++)for(unsigned f=0;f<5;f++)for(async_connect=0;async_connect<2;async_connect++){
  unsigned char out[2100],expected[2100];memset(out,0xa5,sizeof(out));memset(expected,0xa5,sizeof(expected));
  descriptor=fds[f];length=lengths[l];reply_length=replies[r];int capacity=capacities[c];closes=sends=reads=waits=poll_calls=clock_calls=read_calls=0;sent_total=0;
  memset(expected_packet,0,sizeof(expected_packet));memcpy(expected_packet,data,(size_t)length);expected_packet[2052]=(unsigned char)response;
  int result=response?aae_sendIpcMsgAndWaitResp("/tmp/test-ipc",(char *)data,length,(char *)out+8,capacity,5):aae_sendIpcMsg("/tmp/test-ipc",(char *)data,length);
  if(!(result==1&&closes==1&&sends==1&&reads==response&&waits==response+async_connect))fprintf(stderr,"failure result=%d close=%d send=%d read=%d wait=%d response=%d async=%d fd=%d errno=%d\n",result,closes,sends,reads,waits,response,async_connect,descriptor,errno);
  assert(result==1&&closes==1&&sends==1&&reads==response&&waits==response+async_connect);
  if(response){memset(expected+8,0,(size_t)capacity);int copied=reply_length<capacity?reply_length:capacity-1;memcpy(expected+8,reply,(size_t)copied);}
  assert(!memcmp(out,expected,sizeof(out)));
  fprintf(stdout,"%d %d %d %d %d %d %08x\n",response,length,reply_length,capacity,descriptor,async_connect,hash(out,sizeof(out)));cases++;
 }
#ifdef CANDIDATE_ONLY
 unsigned char fault_out[32],untouched[32];memset(untouched,0xa5,sizeof(untouched));
 for(fault=1;fault<=23;fault++){
  descriptor=17;response=1;async_connect=0;length=4;reply_length=6;
  closes=sends=reads=waits=poll_calls=clock_calls=read_calls=0;sent_total=0;fake_ms=100000;
  memset(expected_packet,0,sizeof(expected_packet));memcpy(expected_packet,data,4);expected_packet[2052]=1;
  memcpy(fault_out,untouched,sizeof(fault_out));errno=0;
  int r=aae_sendIpcMsgAndWaitResp("/tmp/test-ipc",(char *)data,4,(char *)fault_out+4,16,5);
  int success=fault==7||fault==12||fault==13||fault==14||fault==17||fault==19||fault==20;
  assert(r==success);assert(closes==((fault==1||fault==21)?0:1));
  if(success){assert(sent_total==2060&&reads==1);assert(!memcmp(fault_out+4,reply,6));for(int i=10;i<20;i++)assert(fault_out[i]==0);}
  else{assert(fault_out[4]==0);assert(!memcmp(fault_out+5,untouched+5,sizeof(fault_out)-5));assert(errno!=0&&errno!=EBADF);}
  assert(!memcmp(fault_out,untouched,4));assert(!memcmp(fault_out+20,untouched+20,12));
 }
 fault=0;closes=sends=0;sent_total=0;clock_calls=0;descriptor=2048;response=0;expected_packet[2052]=0;
 assert(aae_sendIpcMsg("/tmp/test-ipc",(char *)data,4)==1);assert(closes==1&&sent_total==2060);
 int before=closes;char long_path[109];memset(long_path,'x',108);long_path[108]=0;
 assert(!aae_sendIpcMsg(NULL,(char *)data,1)&&errno==EINVAL);
 assert(!aae_sendIpcMsg("",(char *)data,1)&&errno==EINVAL);
 assert(!aae_sendIpcMsg(long_path,(char *)data,1)&&errno==ENAMETOOLONG);
 assert(!aae_sendIpcMsg("/tmp/test-ipc",NULL,1)&&errno==EINVAL);
 assert(!aae_sendIpcMsg("/tmp/test-ipc",(char *)data,-1)&&errno==EINVAL);
 assert(!aae_sendIpcMsg("/tmp/test-ipc",(char *)data,2049)&&errno==EINVAL);
 assert(!aae_sendIpcMsgAndWaitResp("/tmp/test-ipc",(char *)data,4,NULL,16,5)&&errno==EINVAL);
 assert(!aae_sendIpcMsgAndWaitResp("/tmp/test-ipc",(char *)data,4,(char *)fault_out,0,5)&&errno==EINVAL);
 assert(!aae_sendIpcMsgAndWaitResp("/tmp/test-ipc",(char *)data,4,(char *)fault_out,-1,5)&&errno==EINVAL);
 assert(!aae_sendIpcMsgAndWaitResp("/tmp/test-ipc",(char *)data,4,(char *)fault_out,16,0)&&errno==EINVAL);
 assert(!aae_sendIpcMsgAndWaitResp("/tmp/test-ipc",(char *)data,4,(char *)fault_out,16,-1)&&errno==EINVAL);
 assert(closes==before);
 fprintf(stderr,"23 fault scenarios, high descriptor and 11 invalid argument cases passed\n");
#endif
 fprintf(stderr,"%u wire-format/response cases passed\n",cases);return 0;
}
