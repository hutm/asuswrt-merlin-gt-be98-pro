/* SPDX-License-Identifier: GPL-2.0-only */
#define _GNU_SOURCE
#include "migration.h"
#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
struct nvram_tuple router_defaults[]={
 {"plain",NULL,0,0,0,0,NULL},{"encrypted",NULL,0,0,0,1,NULL},
 {"short",NULL,0,0,0,0,NULL},{"fallback",NULL,0,0,0,2,NULL},{NULL,NULL,0,0,0,0,NULL}};
static char *version,*mode;
static int has_salt,quiet,fail_step,step,commits,deny,random_fail;
static int event(const char *name,const char *value){++step;if(!quiet)printf("%s %s\n",name,value);return step==fail_step?-1:0;}
char *nvram_get(const char *name){return !strcmp(name,"short")?"small":"123456789012345678901234";}
char *nvram_default_get(const char *name){return !strcmp(name,"nvram_encrypt_ver")?version:mode;}
char *nvram_get_salt(void){return deny?(char *)(intptr_t)-1:has_salt?"existing":NULL;}
int nvram_set(const char *name,const char *value){return event(name,value);}
int nvram_set_int(const char *name,int value){char text[32];snprintf(text,sizeof text,"%d",value);return nvram_set(name,text);}
int nvram_commit(void){++commits;return event("commit","");}
int start_enc_nvram(void){return event("encrypt","");}
int start_dec_nvram(void){return event("decrypt","");}
int dec_nvram(char *name,char *input,char *output){(void)input;if(event("decode",name)<0)return -1;strcpy(output,"decoded");return strcmp(name,"fallback")!=0;}
int RAND_bytes(unsigned char *out,int size){assert(size==16);if(random_fail)return 0;for(int i=0;i<size;++i)out[i]=(unsigned char)(i*13);return 1;}
#ifdef REFERENCE
#include <dlfcn.h>
FILE *fopen64(const char *path,const char *access){assert(!strcmp(path,"/dev/urandom")&&!strcmp(access,"r"));FILE *f=tmpfile();assert(f);for(int i=0;i<17;++i)fputc(i*13,f);rewind(f);return f;}
#endif
int main(void){
#ifdef REFERENCE
 assert(dlopen("libnvram.so",RTLD_LAZY|RTLD_GLOBAL));void *h=dlopen("libshared.so",RTLD_NOW);assert(h);int (*init)(void)=dlsym(h,"init_enc_nvram");assert(init);
#else
 int (*init)(void)=init_enc_nvram;
#endif
 char *versions[]={NULL,"2","3","-1","nonnumeric"};
 for(unsigned v=0;v<5;++v)for(has_salt=0;has_salt<2;++has_salt)for(unsigned m=0;m<2;++m){
  version=versions[v];mode=m?"1":NULL;step=commits=0;printf("C %u %d %u\n",v,has_salt,m);assert(!init()&&commits==1);
 }
#ifndef REFERENCE
 quiet=1;version="3";mode="1";has_salt=0;
 step=commits=0;assert(!init());int total=step;
 for(fail_step=1;fail_step<=total;++fail_step){step=commits=0;assert(init()==-1&&step==fail_step);}
 fail_step=0;deny=1;step=commits=0;assert(init()==-1&&errno==EACCES&&!commits);deny=0;
 random_fail=1;step=commits=0;assert(init()==-1&&errno==EIO&&!commits);random_fail=0;
 version="999999999999999999999999";step=commits=0;assert(init()==-1&&errno==ERANGE&&!commits);
#endif
 return 0;
}
