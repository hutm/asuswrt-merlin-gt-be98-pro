/* SPDX-License-Identifier: GPL-2.0-only */
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
extern int aae_sendIpcMsg(char *,char *,int);
extern int aae_sendIpcMsgAndWaitResp(char *,char *,int,char *,int,int);
static void pause_ms(long ms){struct timespec t={ms/1000,(ms%1000)*1000000};while(nanosleep(&t,&t))assert(errno==EINTR);}
static void run_case(int mode,int length,int capacity)
{
 char path[100];snprintf(path,sizeof(path),"/tmp/source-ipc-%ld.sock",(long)getpid());
 unsigned char data[2048],packet[2060]={0},reply[2060]={0},out[2100],expected[2100];
 for(int i=0;i<2048;i++)data[i]=(unsigned char)(i*137+19);
 memcpy(packet,data,(size_t)length);packet[2052]=mode!=0;
 memcpy(reply,"{\"status\":\"ok\"}",15);
 memset(out,0xa5,sizeof(out));memcpy(expected,out,sizeof(out));
 int listener=socket(AF_UNIX,SOCK_STREAM,0);assert(listener>=0);
 struct sockaddr_un addr={.sun_family=AF_UNIX};strcpy(addr.sun_path,path);
 assert(!bind(listener,(struct sockaddr *)&addr,sizeof(addr)));assert(!listen(listener,1));
 pid_t child=fork();assert(child>=0);
 if(!child){
  int fd=accept(listener,NULL,NULL);assert(fd>=0);close(listener);
  unsigned char incoming[2060];size_t used=0;
  while(used<sizeof(incoming)){ssize_t n=read(fd,incoming+used,sizeof(incoming)-used);assert(n>0);used+=(size_t)n;}
  assert(!memcmp(incoming,packet,sizeof(packet)));
  if(mode==2)pause_ms(150);
  if(mode==4)pause_ms(1500);
  if(mode==1||mode==2||mode==3){
   size_t amount=mode==3?5:sizeof(reply);size_t sent=0;
   while(sent<amount){ssize_t n=send(fd,reply+sent,amount-sent,MSG_NOSIGNAL);assert(n>0);sent+=(size_t)n;}
  }
  close(fd);_exit(0);
 }
 close(listener);errno=0;
 int result=mode?aae_sendIpcMsgAndWaitResp(path,(char *)data,length,(char *)out+8,capacity,1):aae_sendIpcMsg(path,(char *)data,length);
 int saved=errno;
 if(mode==4){assert(!result&&saved==ETIMEDOUT);expected[8]=0;}
 else {
  assert(result==1);
  if(mode){size_t n=mode==3?5:mode==5?0:sizeof(reply);size_t copied=n<(size_t)capacity?n:(size_t)capacity-1;memset(expected+8,0,(size_t)capacity);memcpy(expected+8,reply,copied);}
 }
 assert(!memcmp(out,expected,sizeof(out)));
 int status;assert(waitpid(child,&status,0)==child);assert(WIFEXITED(status)&&WEXITSTATUS(status)==0);assert(!unlink(path));
 fprintf(stderr,"socket mode=%d request=%d capacity=%d passed\n",mode,length,capacity);
}
int main(void)
{
 int lengths[]={0,1,2047,2048},capacities[]={1,2,16,2048,2071};
 for(unsigned l=0;l<4;l++)for(unsigned c=0;c<5;c++)run_case(1,lengths[l],capacities[c]);
 run_case(0,2048,16);run_case(2,15,2048);run_case(3,15,2048);run_case(4,15,2048);run_case(5,15,2048);
 char output[16];memset(output,0xa5,sizeof(output));errno=0;
 assert(!aae_sendIpcMsgAndWaitResp("/tmp/no-such-source-ipc-endpoint","hi",2,output,sizeof(output),1));assert(errno==ENOENT);
 assert(output[0]==0);for(unsigned i=1;i<sizeof(output);i++)assert((unsigned char)output[i]==0xa5);
 fprintf(stderr,"26 real Unix-socket cases passed\n");return 0;
}
