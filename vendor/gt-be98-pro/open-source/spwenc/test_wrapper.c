/* SPDX-License-Identifier: GPL-2.0-only */
#include "wrapper.h"
#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
static int wrong,denied,reads;
char *nvram_get(const char *name){assert(!strcmp(name,"productid"));++reads;return "TEST-router";}
char *nvram_get_salt(void){++reads;return denied?(char *)(intptr_t)-1:wrong?"different":"synthetic-salt";}
char *get_lan_hwaddr(void){++reads;return "02:11:22:33:44:55";}
#ifdef REFERENCE
#include <dlfcn.h>
#endif
int main(void){
#ifdef REFERENCE
 assert(dlopen("libnvram.so",RTLD_LAZY|RTLD_GLOBAL));void *h=dlopen("libshared.so",RTLD_NOW);assert(h);
 int (*enc)(const char *,char *,int)=dlsym(h,"pw_enc");
 int (*dec)(const char *,char *,int,int)=dlsym(h,"pw_dec");assert(enc&&dec);
#else
 int (*enc)(const char *,char *,int)=pw_enc;
 int (*dec)(const char *,char *,int,int)=pw_dec;
#endif
 char plain[129],encoded[256],out[256];
 for(unsigned length=0;length<129;++length)for(int salted=0;salted<2;++salted){
  memset(plain,'a'+length%26,length);plain[length]=0;reads=0;
  assert(enc(plain,encoded,salted)==0);
  assert(dec(encoded,out,sizeof out,salted)==(length?1:0));assert(!strcmp(plain,out));
  if(!length)assert(!reads);
  printf("%u %d %s %s\n",length,salted,encoded,out);
  if(length&&salted){
   wrong=1;assert(dec(encoded,out,sizeof out,1)==0&&!strcmp(encoded,out));
   /* Keep reference scratch allocation at least as large as decoded bytes. */
   int capacity=(int)strlen(encoded)-1;
   assert(dec(encoded,out,capacity,1)==0);
   assert(strlen(out)==(size_t)capacity-1&&!memcmp(out,encoded,(size_t)capacity-1));wrong=0;
  }
 }
#ifndef REFERENCE
 strcpy(out,"unchanged");denied=1;
 assert(dec(encoded,out,sizeof out,1)==-1&&errno==EACCES&&!strcmp(out,"unchanged"));
 assert(enc("a",out,1)==-1&&errno==EACCES&&!strcmp(out,"unchanged"));denied=0;
 assert(dec(encoded,out,1,1)==-1&&errno==ENOSPC&&!strcmp(out,"unchanged"));
 assert(dec(encoded,out,0,1)==-1&&errno==EINVAL);
 assert(enc(NULL,out,0)==-1&&errno==EINVAL);
 assert(dec("not encrypted",out,sizeof out,0)==0&&!strcmp(out,"not encrypted"));
#endif
 return 0;
}
