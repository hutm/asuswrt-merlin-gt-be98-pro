/* SPDX-License-Identifier: GPL-2.0-only */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#ifdef REFERENCE
#include <dlfcn.h>
#else
#include "names.h"
#endif
int main(void){
#ifdef REFERENCE
 assert(dlopen("libnvram.so",RTLD_LAZY|RTLD_GLOBAL));void *h=dlopen("libshared.so",RTLD_NOW);assert(h);
 int (*get)(int,char *,size_t)=dlsym(h,"webapi_get_b");
 void (*append)(char *,size_t,...)=dlsym(h,"c");assert(get&&append);
#else
 int (*get)(int,char *,size_t)=webapi_get_b;
 void (*append)(char *,size_t,...)=c;
#endif
 char out[64];
 for(int id=-1;id<=13;++id)for(size_t size=1;size<=33;++size){
  memset(out,0xa5,sizeof out);int n=get(id,out,size);printf("N %d %zu %d %s\n",id,size,n,out);
  assert(out[size]==(char)0xa5);assert(n==(int)strlen(out));
 }
 for(size_t size=4;size<=16;++size){
  memset(out,0xa5,sizeof out);strcpy(out,"abc");append(out,size,'X',0x159,'Z',0);
  printf("C %zu %s\n",size,out);assert(out[size]==(char)0xa5);
 }
 assert(!get(3,NULL,32));append(NULL,32,'a',0);
#ifndef REFERENCE
 memset(out,0xa5,sizeof out);assert(!get(3,out,0)&&out[0]==(char)0xa5);
 append(out,sizeof out,'a',0);for(unsigned i=0;i<sizeof out;++i)assert(out[i]==(char)0xa5);
#endif
 return 0;
}
