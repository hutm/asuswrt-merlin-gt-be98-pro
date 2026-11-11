/* SPDX-License-Identifier: GPL-2.0-only */
#include "migration.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
struct nvram_tuple router_defaults[]={
 {"first",NULL,0,0,0,1,NULL},{"skip",NULL,0,0,0,0,NULL},
 {"second",NULL,0,0,0,1,NULL},{"other",NULL,0,0,0,2,NULL},{NULL,NULL,0,0,0,0,NULL}};
static int writes,fail_convert,fail_write,quiet,missing;
char *nvram_get(const char *name){return missing?NULL:!strcmp(name,"first")?"value-a":"value-b";}
int nvram_set(const char *name,const char *value){++writes;if(!quiet)printf("W %s %s\n",name,value);return writes==fail_write?-1:0;}
static int convert(char *name,char *input,char *output,int dec){
 if(fail_convert&&!strcmp(name,"second")){errno=EACCES;return -1;}
 snprintf(output,4096,"%c:%s",dec?'D':'E',input);return 1;
}
int enc_nvram(char *name,char *input,char *output){return convert(name,input,output,0);}
int dec_nvram(char *name,char *input,char *output){return convert(name,input,output,1);}
#ifdef REFERENCE
#include <dlfcn.h>
#endif
int main(void){
#ifdef REFERENCE
 assert(dlopen("libnvram.so",RTLD_LAZY|RTLD_GLOBAL));void *h=dlopen("libshared.so",RTLD_NOW);assert(h);
 int (*enc)(void)=dlsym(h,"start_enc_nvram"),(*dec)(void)=dlsym(h,"start_dec_nvram");assert(enc&&dec);
#else
 int (*enc)(void)=start_enc_nvram,(*dec)(void)=start_dec_nvram;
#endif
 for(missing=0;missing<2;++missing)for(unsigned mask=0;mask<4;++mask){
  router_defaults[0].enc=mask&1;router_defaults[2].enc=(mask>>1)&1;
  for(int mode=0;mode<2;++mode){writes=0;printf("C %d %u %d\n",missing,mask,mode);assert((mode?dec():enc())==0);assert(writes==1+(int)(mask&1)+(int)((mask>>1)&1));}
 }
#ifndef REFERENCE
 quiet=1;missing=0;router_defaults[0].enc=router_defaults[2].enc=1;
 for(int mode=0;mode<2;++mode){
  writes=0;fail_convert=1;assert((mode?dec():enc())==-1&&errno==EACCES&&!writes);fail_convert=0;
  for(fail_write=1;fail_write<=3;++fail_write){writes=0;assert((mode?dec():enc())==-1&&errno==EIO&&writes==fail_write);}fail_write=0;
 }
#endif
 return 0;
}
