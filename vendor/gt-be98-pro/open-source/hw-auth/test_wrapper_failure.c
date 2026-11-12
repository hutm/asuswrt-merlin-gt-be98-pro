/* SPDX-License-Identifier: GPL-2.0-only */
#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
static int failure;static time_t stamp;static int good;static unsigned hashes;
time_t time(time_t *out){assert(!out);puts("time");return stamp;}
char *get_auth_code(char *in,char *out,int cap){assert(cap==65);if((failure==1&&hashes==0)||(failure==2&&hashes==1)){hashes++;return NULL;}printf("hash [%s]\n",in);snprintf(out,cap,"%s",hashes++?"expected":"request");return out;}
char *hw_auth_check(char *id,char *code,time_t timestamp,char *out,int cap){if(failure==3)return NULL;assert(!strcmp(code,"request")&&timestamp==stamp&&cap==65);printf("peer [%s] %ld\n",id,(long)timestamp);snprintf(out,cap,"%s",good?"expected":"wrong");return out;}
int main(int argc,char **argv){
 assert(argc==2);void *h=dlopen(argv[1],RTLD_LAZY);if(!h){fputs(dlerror(),stderr);return 1;}int (*fn)(char *,char *)=dlsym(h,"auth_validate");assert(fn);
 stamp=1700000000;good=1;
 for(failure=1;failure<=3;failure++){hashes=0;assert(fn("id","key")==0);}
 failure=0;hashes=0;assert(!fn(NULL,"key"));assert(!fn("id",NULL));assert(!hashes);
 puts("5 wrapper failure cases passed");return 0;
}
