#define _GNU_SOURCE
#include <assert.h>
#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <limits.h>
static int instance,driver_result,set_result,apply,open_fail,close_result,reads,sets;
static const char *channel,*source,*old,*output;static char stored[1024];
int cprintf(const char *fmt,...){va_list ap;va_start(ap,fmt);int n=vprintf(fmt,ap);va_end(ap);return n;}
char *nvram_get(const char *key){const char*v=strstr(key,"chanspec")?channel:strstr(key,"valid")?(apply?stored:old):source;reads++;printf("get:%s:%s;",key,v?v:"NULL");return(char*)v;}
int nvram_set(const char*key,const char*value){sets++;printf("set:%s:%s:%d;",key,value,set_result);snprintf(stored,sizeof(stored),"%s",value);return set_result;}
int wl_ioctl(char*ifname,int cmd,void*buf,int len){assert(cmd==14&&len==4);printf("query:%s:%d:%d;",ifname,instance,driver_result);memcpy(buf,&instance,4);return driver_result;}
FILE *popen(const char*cmd,const char*type){assert(!strcmp(cmd,"acs_cli2 -C")&&!strcmp(type,"r"));printf("open:%d;",open_fail);if(open_fail)return NULL;FILE*fp=tmpfile();assert(fp);assert(fwrite(output,1,strlen(output),fp)==strlen(output));rewind(fp);return fp;}
int pclose(FILE*fp){assert(fp);assert(!fclose(fp));printf("close:%d;",close_result);return close_result;}
int main(int argc,char**argv){assert(argc==2||argc==3);void*h=dlopen(argv[1],RTLD_LAZY|RTLD_LOCAL);if(!h){puts(dlerror());return 1;}int(*reset)(char*)=dlsym(h,"avbl_reset_exclvalid");int(*check)(char*)=dlsym(h,"chk_acscli2_cmds");assert(reset&&check);
if(argc==3){reads=sets=0;assert(reset(NULL)==-1&&check(NULL)==0);assert(!reads&&!sets);channel="0";source="1,2";old="old";apply=1;driver_result=0;instance=INT_MIN;assert(reset("eth0")==0);instance=INT_MAX;assert(reset("eth0")==0);puts("4 channel settings boundary cases passed");return 0;}
const char *channels[]={NULL,"","0","00","1","auto"},*values[]={NULL,"","1","0x123,0x456"," spaces \n"},*names[]={"","eth0","wl0.1"};int units[]={-1,0,1,3,15};unsigned count=0;
for(unsigned u=0;u<5;u++)for(unsigned n=0;n<3;n++)for(driver_result=-1;driver_result<=1;driver_result++)for(unsigned c=0;c<6;c++)for(unsigned s=0;s<5;s++)for(unsigned o=0;o<5;o++)for(set_result=-1;set_result<=1;set_result++)for(apply=0;apply<2;apply++){
instance=units[u];channel=channels[c];source=values[s];old=values[o];reads=sets=0;stored[0]=0;printf("reset:%u;",count++);int rc=reset((char*)names[n]);printf("result:%d;reads:%d;sets:%d\n",rc,reads,sets);}
char long_output[256];memset(long_output,'x',sizeof(long_output));memcpy(long_output+62,"target",6);long_output[255]=0;
const char *commands[]={"","target","foo","foo bar","zz","x","very_long_command_name_that_truncates_the_settings_key"},*outputs[]={"","target\n","foo target bar\n","foo\nbar\ntarget\n","targeting\n",long_output,"no match\n","foo bar\n"};
for(unsigned c=0;c<7;c++)for(unsigned o=0;o<8;o++)for(open_fail=0;open_fail<2;open_fail++)for(close_result=-1;close_result<=1;close_result++)for(set_result=-1;set_result<=1;set_result++){
output=outputs[o];reads=sets=0;printf("check:%u;",count++);int rc=check((char*)commands[c]);assert(sets==1&&!reads);printf("result:%d\n",rc);}
printf("%u channel settings comparisons\n",count);return 0;}
