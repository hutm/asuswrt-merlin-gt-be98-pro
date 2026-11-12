/* SPDX-License-Identifier: GPL-2.0-only */
#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
static time_t stamp;static int good;static unsigned hashes;
time_t time(time_t *out){assert(!out);puts("time");return stamp;}
char *get_auth_code(char *in,char *out,int cap){assert(cap==65);printf("hash [%s]\n",in);snprintf(out,cap,"%s",hashes++?"expected":"request");return out;}
char *hw_auth_check(char *id,char *code,time_t timestamp,char *out,int cap){assert(!strcmp(code,"request")&&timestamp==stamp&&cap==65);printf("peer [%s] %ld\n",id,(long)timestamp);snprintf(out,cap,"%s",good?"expected":"wrong");return out;}
int main(int argc,char **argv){
 assert(argc==2);void *h=dlopen(argv[1],RTLD_LAZY);if(!h){fputs(dlerror(),stderr);return 1;}int (*fn)(char *,char *)=dlsym(h,"auth_validate");assert(fn);
 unsigned lengths[]={0,1,31,32,63,64,126,127,150};time_t times[]={-1,0,1700000000};unsigned cases=0;
 for(unsigned a=0;a<9;a++)for(unsigned b=0;b<9;b++)for(unsigned t=0;t<3;t++)for(good=0;good<2;good++){
  char id[151],key[151];memset(id,'i',lengths[a]);id[lengths[a]]=0;memset(key,'k',lengths[b]);key[lengths[b]]=0;stamp=times[t];hashes=0;
  printf("CASE %u %u %u %d\n",a,b,t,good);assert(fn(id,key)==good);assert(hashes==2);cases++;
 }
 fprintf(stderr,"%u public wrapper cases passed\n",cases);return 0;
}
