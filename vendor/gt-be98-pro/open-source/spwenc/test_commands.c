/* SPDX-License-Identifier: GPL-2.0-only */
#include "commands.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
void dbg(const char *format,...){(void)format;}
static void hex(const char *s){if(!s){printf("NULL");return;}for(;*s;++s)printf("%02x",(unsigned char)*s);}
int _eval(char *const argv[],const char *file,int timeout,pid_t *pid){assert(!timeout);printf("E %d ",!!pid);hex(file);for(unsigned i=0;argv[i];++i){putchar(' ');hex(argv[i]);}putchar('\n');if(pid)*pid=123;return 37;}
#ifdef REFERENCE
#include <dlfcn.h>
#endif
int main(void){
#ifdef REFERENCE
 assert(dlopen("libnvram.so",RTLD_LAZY|RTLD_GLOBAL));void *h=dlopen("libshared.so",RTLD_NOW);assert(h);
 int (*sync)(const char *,...)=dlsym(h,"safe_do_system"),(*async)(const char *,...)=dlsym(h,"safe_fork_do_system");
 int (*file)(const char *,int,const char *,...)=dlsym(h,"safe_do_system_to_file");assert(sync&&async&&file);
#else
 int (*sync)(const char *,...)=safe_do_system,(*async)(const char *,...)=safe_fork_do_system;
 int (*file)(const char *,int,const char *,...)=safe_do_system_to_file;
#endif
 const char *cases[]={""," ","echo a b","echo 'a b'","echo \"a b\"","echo '' x","''","echo a\\ b","echo 'a\\ b'","echo \"a'b\"","echo 'a\"b'","echo a'bc'd","echo \\","echo 'bad","echo \"bad","sh -c x","/bin/BASH -c x","dash","ash","ksh","busybox sh -c x","/usr/bin/echo a\tb\nc","echo a;b","echo 'a;b'","echo \\;","echo $(x)","echo * ? [a] # ~"};
 for(unsigned i=0;i<sizeof cases/sizeof cases[0];++i)for(unsigned mode=0;mode<4;++mode){
  printf("C %u %u\n",i,mode);errno=0;int r=mode==0?sync("%s",cases[i]):mode==1?async("%s",cases[i]):file("/tmp/synthetic output",mode==3,"%s",cases[i]);printf("R %d %d\n",r,r<0?errno:0);
 }
 for(unsigned ch=1;ch<127;++ch)for(unsigned quote=0;quote<4;++quote){
  char text[32];if(quote==0)snprintf(text,sizeof text,"echo a%cb",ch);else if(quote==1)snprintf(text,sizeof text,"echo 'a%cb'",ch);else if(quote==2)snprintf(text,sizeof text,"echo \"a%cb\"",ch);else snprintf(text,sizeof text,"echo a\\%cb",ch);
  printf("A %u %u\n",ch,quote);errno=0;int r=sync("%s",text);printf("R %d %d\n",r,r<0?errno:0);
 }
 char large[8200];unsigned lengths[]={127,128,129,8190,8191,8192};
 for(unsigned i=0;i<6;++i){memset(large,'a',lengths[i]);large[lengths[i]]=0;printf("L %u\n",lengths[i]);errno=0;int r=sync("echo %s",large);printf("R %d %d\n",r,r<0?errno:0);}
 char path[1100];memset(path,'p',sizeof path-1);path[sizeof path-1]=0;
 const char *paths[]={NULL,"",path};
 for(unsigned i=0;i<3;++i)for(int append=-1;append<=1;++append){
  printf("F %u %d\n",i,append);assert(file(paths[i],append,"echo %d %s",42,"a b c d e f g h i j k l m n o p q r s t")==37);
 }
 assert(sync(NULL)==-1&&errno==EINVAL);assert(async(NULL)==-1&&errno==EINVAL);assert(file(NULL,0,NULL)==-1&&errno==EINVAL);
 return 0;
}
