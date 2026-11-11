/* SPDX-License-Identifier: GPL-2.0-only */
#include "commands.h"
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
extern int _eval(char *const [],const char *,int,pid_t *);
static int tokenize(const char *input,char *storage,char **argv)
{
 size_t count=0;char *out=storage;
 while(*input){
  while(isspace((unsigned char)*input))++input;
  if(!*input)break;
  char *start=out;int single=0,dbl=0;
  while(*input){
   unsigned char ch=(unsigned char)*input;
   if(!single&&!dbl&&isspace(ch))break;
   if(ch=='\''&&!dbl){single=!single;++input;continue;}
   if(ch=='"'&&!single){dbl=!dbl;++input;continue;}
   if(ch=='\\'){if(!input[1]){errno=EINVAL;return -1;}ch=(unsigned char)input[1];input+=2;}
   else{
    if(!single&&!dbl&&strchr("$&();<>`|",ch)){errno=EINVAL;return -1;}
    ++input;
   }
   if((size_t)(out-start)>=8190){errno=E2BIG;return -1;}
   *out++=(char)ch;
  }
  if(single||dbl){errno=EINVAL;return -1;}
  if(out!=start){*out++=0;argv[count++]=start;}
 }
 argv[count]=NULL;if(!count){errno=EINVAL;return -1;}return 0;
}
static int run(const char *file,int append,int async,const char *format,va_list args)
{
 if(!format){errno=EINVAL;return -1;}
 va_list copy;va_copy(copy,args);int n=vsnprintf(NULL,0,format,copy);va_end(copy);
 if(n<0){errno=ENOMEM;return -1;}
 size_t size=(size_t)n+1;char *command=malloc(size),*storage=malloc(size);
 char **argv=calloc((size_t)n/2+2,sizeof *argv);int result=-1;
 if(!command||!storage||!argv){errno=ENOMEM;goto done;}
 if(vsnprintf(command,size,format,args)<0){errno=ENOMEM;goto done;}
 if(tokenize(command,storage,argv)<0)goto done;
 const char *base=strrchr(argv[0],'/');base=base?base+1:argv[0];
 const char *shells[]={"sh","bash","dash","ash","ksh"};
 for(unsigned i=0;i<5;++i)if(!strcasecmp(base,shells[i])){errno=EPERM;goto done;}
 char redirect[1024];const char *path=NULL;
 if(file){snprintf(redirect,sizeof redirect,append?">>%s":">%s",file);path=redirect;}
 pid_t pid=0;result=_eval(argv,path,0,async?&pid:NULL);
done:
 {int error=errno;free(argv);free(storage);free(command);errno=error;}
 return result;
}
int safe_do_system(const char *format,...){va_list args;va_start(args,format);int r=run(NULL,0,0,format,args);va_end(args);return r;}
int safe_fork_do_system(const char *format,...){va_list args;va_start(args,format);int r=run(NULL,0,1,format,args);va_end(args);return r;}
int safe_do_system_to_file(const char *file,int append,const char *format,...){va_list args;va_start(args,format);int r=run(file,append,0,format,args);va_end(args);return r;}
