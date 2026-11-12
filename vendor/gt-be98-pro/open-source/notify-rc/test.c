/* SPDX-License-Identifier: GPL-2.0-only */
#include <assert.h>
#include <stdint.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
extern int notify_rc(const char *);
extern int notify_rc_after_wait(const char *);
extern int notify_rc_after_period_wait(const char *, int);
extern int notify_rc_and_wait(const char *);
extern int notify_rc_and_wait_1min(const char *);
extern int notify_rc_and_wait_2min(const char *);
extern int notify_rc_and_period_wait(const char *, int);
extern int wait_rc_service(int, const char *);
extern int skippable_event(const char *);
extern char *skip_table;
static char service[512],last[512],pidvalue[32],reason[128];
static int valid, before_clear,after_clear,sleeps,signals,sets,evals;
static uint32_t trace;
static unsigned missing,failures,action,phase_sleeps,race_mode,rc_reads;
static void record(const char *s){for(;*s;s++)trace=(trace^(unsigned char)*s)*16777619u;trace=(trace^255u)*16777619u;}
size_t strlcpy(char *dst,const char *src,size_t n){size_t len=strlen(src);if(n){size_t k=len<n-1?len:n-1;memcpy(dst,src,k);dst[k]=0;}return len;}
static char *storage(const char *key,unsigned *bit){
 if(!strcmp(key,"rc_service")){*bit=1;return service;}
 if(!strcmp(key,"last_rc_service")){*bit=2;return last;}
 if(!strcmp(key,"rc_service_pid")){*bit=4;return pidvalue;}
 if(!strcmp(key,"sys_reboot_reason")){*bit=8;return reason;}
 assert(0);return NULL;
}
char *nvram_get(const char *key){unsigned bit;record(key);char *p=storage(key,&bit);
 if(bit==1&&signals){rc_reads++;
  if((race_mode==1&&rc_reads==2)||(race_mode==4&&rc_reads==1)||(race_mode==5&&rc_reads>=2))return NULL;
  if(race_mode==2&&rc_reads==2)return "replacement";
  if(race_mode==3&&rc_reads==2)return "";
 }
 return missing&bit?NULL:p;}
int nvram_get_int(const char *key){char *p=nvram_get(key);return p?atoi(p):0;}
int nvram_set(const char *key,const char *val){
 unsigned bit;sets++;record("set");record(key);record(val);
 if(failures&1)return -1;
 char *dst=storage(key,&bit);missing&=~bit;
 size_t cap=dst==pidvalue?sizeof(pidvalue):dst==reason?sizeof(reason):sizeof(service);
 strlcpy(dst,val,cap);return 0;
}
int nvram_set_int(const char *key,int v){char s[32];snprintf(s,sizeof(s),"%d",v);return nvram_set(key,s);}
pid_t getpid(void){return 321;}
char *psname(int pid,char *name,int cap){(void)pid;strlcpy(name,"test-process",cap);return name;}
int validate_rc_service(const char *s){record("validate");record(s);return valid;}
int cprintf(const char *fmt,...){(void)fmt;return 0;}
void logmessage_normal(const char *tag,const char *fmt,...){(void)tag;(void)fmt;}
int kill(pid_t pid,int sig){assert(pid==1&&sig==SIGUSR1);signals++;phase_sleeps=0;record("signal");if(failures&2)return -1;if(after_clear==0)service[0]=0;return 0;}
unsigned int sleep(unsigned int s){
 assert(s==1);record("sleep");assert(++sleeps<1000);phase_sleeps++;
 int limit=signals?after_clear:before_clear;
 if(limit>=0&&phase_sleeps>=(unsigned)limit){
  switch(action){
   case 0:service[0]=0;break;
   case 1:strlcpy(last,service,sizeof(last));break;
   case 2:strlcpy(service,"replacement",sizeof(service));break;
   case 3:missing|=1;break;
   case 4:service[0]=0;missing|=2;break;
  }
 }
 return 0;
}
int _eval(char *const argv[],const char *path,int timeout,pid_t *pid){assert(!path&&!timeout&&!pid);assert(argv&&argv[0]&&argv[1]&&!argv[2]);assert(!strcmp(argv[0],"killall")&&!strcmp(argv[1],"sh"));record("eval");evals++;return failures&4?-1:0;}
static void reset(const char *busy,const char *ack,int clear,int after){strlcpy(service,busy,sizeof(service));strlcpy(last,ack,sizeof(last));strlcpy(pidvalue,"123",sizeof(pidvalue));reason[0]=0;valid=1;missing=failures=action=phase_sleeps=race_mode=rc_reads=0;before_clear=clear;after_clear=after;sleeps=signals=sets=evals=0;trace=2166136261u;}
static int invoke(int api,const char *event,int period){switch(api){case 0:return notify_rc(event);case 1:return notify_rc_after_wait(event);case 2:return notify_rc_after_period_wait(event,period);case 3:return notify_rc_and_wait(event);case 4:return notify_rc_and_wait_1min(event);case 5:return notify_rc_and_wait_2min(event);case 6:return notify_rc_and_period_wait(event,period);default:return wait_rc_service(period,event);}}
int main(void){
 const char *events[]={NULL,"","restart_autowan","restart_dnsmasq","reboot","prefix_reboot_suffix"};
 const char *busy[]={"","other","restart_autowan"};
 int periods[]={-1,0,1,6,7,8,15};int clear[]={-1,1,6,7};unsigned count=0;
 assert(skip_table&&!strcmp(skip_table,"restart_autowan"));for(unsigned e=0;e<6;e++)printf("skip %u %d\n",e,skippable_event(events[e]));
 for(int api=0;api<8;api++)for(unsigned e=0;e<6;e++)for(unsigned b=0;b<3;b++)for(int ack=0;ack<2;ack++)for(unsigned p=0;p<7;p++)for(unsigned c=0;c<4;c++){
  reset(busy[b],ack?busy[b]:"",clear[c],clear[c]);int r=invoke(api,events[e],periods[p]);
  if(api<7&&(!events[e]||!*events[e]||((api==2||api==6)&&periods[p]<0)))assert(r==-1&&!signals&&!sets&&!evals&&!sleeps);
  printf("%d %u %u %d %d %d: %d %d %d %d %d %08x [%s] [%s] [%s]\n",api,e,b,ack,periods[p],clear[c],r,sleeps,signals,sets,evals,trace,service,pidvalue,reason);count++;
 }
 for(int api=0;api<7;api++){reset("","",-1,-1);valid=0;assert(invoke(api,"restart_dnsmasq",5)==-1);assert(!signals&&!sets&&!sleeps&&!evals);}

 unsigned edges=0;
 char long_event[384],long_ack[384];memset(long_event,'a',sizeof(long_event)-1);long_event[sizeof(long_event)-1]=0;
 strcpy(long_ack,long_event);long_ack[126]='b';
 const char *requests[]={"restart_dnsmasq","restart_autowan","reboot",long_event};
 const char *acks[]={"",long_event,long_ack,"other-extra"};
 int delays[]={-1,0,1,7,15};
 for(int api=2;api<=7;api+=api==2?4:1)for(unsigned req=0;req<4;req++)for(unsigned ack=0;ack<4;ack++)for(unsigned mask=0;mask<8;mask++)for(unsigned failure=0;failure<8;failure++)for(unsigned mode=0;mode<5;mode++){
  reset(req==3?long_event:"other",acks[ack],delays[mode],delays[(mode+1)%5]);missing=mask;failures=failure;action=mode;
  int r=invoke(api,requests[req],8);
  printf("edge %d %u %u %u %u %u: %d %d %d %d %d %08x [%s] [%s] [%s]\n",api,req,ack,mask,failure,mode,r,sleeps,signals,sets,evals,trace,service,pidvalue,reason);edges++;
 }
 for(int api=2;api<=7;api+=api==2?4:1){reset("other","other",1,1);int r=invoke(api,"restart_dnsmasq",INT_MAX);printf("max %d %d %d %08x\n",api,r,sleeps,trace);}
 for(unsigned mode=1;mode<=5;mode++)for(int wait=0;wait<=3;wait++){
  reset("","",-1,-1);race_mode=mode;int r=notify_rc_and_period_wait("restart_dnsmasq",wait);
  printf("race %u %d: %d %d %u %08x\n",mode,wait,r,sleeps,rc_reads,trace);
  if(wait==1&&mode<=3)assert(r==2);
  if(wait==0&&mode==4)assert(r==2);
 }
 char *original_skip=skip_table;skip_table="custom_skip";assert(skippable_event("custom_skip"));assert(!skippable_event("restart_autowan"));skip_table=original_skip;
#ifdef CANDIDATE_ONLY
 reset("busy","",1,1);assert(wait_rc_service(INT_MIN,"restart_dnsmasq")==0);assert(!sleeps&&!sets&&!signals);
#endif
 fprintf(stderr,"%u additional state/failure cases passed\n",edges);
 fprintf(stderr,"%u notification scenarios, 6 skip cases, 7 validation failures passed\n",count);
 return 0;
}
