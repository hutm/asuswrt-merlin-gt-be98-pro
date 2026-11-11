/* SPDX-License-Identifier: GPL-2.0-only */
#define _GNU_SOURCE
#include "migration.h"
#include "settings.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
struct nvram_tuple router_defaults[]={
 {"http_passwd",NULL,0,0,0,1,NULL},{"acc_list",NULL,0,0,0,1,NULL},
 {"former_secret",NULL,0,0,0,0,NULL},{"ordinary",NULL,0,0,0,0,NULL},
 {NULL,NULL,0,0,0,0,NULL}};
static const char *names[]={"http_passwd","acc_list","former_secret","ordinary","nvram_salt","nvram_encrypt","nvram_encrypt_ver"};
static char values[7][4096];static unsigned present[7],commits;
static char *version;
char *nvram_get(const char *name){
 if(!strcmp(name,"productid"))return "TEST-router";
 for(unsigned i=0;i<7;++i)if(!strcmp(name,names[i]))return present[i]?values[i]:NULL;
 assert(0);return NULL;
}
int nvram_set(const char *name,const char *value){
 for(unsigned i=0;i<7;++i)if(!strcmp(name,names[i])){assert(strlen(value)<sizeof values[i]);memmove(values[i],value,strlen(value)+1);present[i]=1;return 0;}
 assert(0);return -1;
}
int nvram_set_int(const char *name,int value){char text[32];snprintf(text,sizeof text,"%d",value);return nvram_set(name,text);}
int nvram_commit(void){++commits;return 0;}
char *nvram_default_get(const char *name){if(!strcmp(name,"nvram_encrypt_ver"))return version;assert(!strcmp(name,"nvram_encrypt"));return "1";}
char *nvram_get_salt(void){return nvram_get("nvram_salt");}
char *get_lan_hwaddr(void){return "02:11:22:33:44:55";}
int RAND_bytes(unsigned char *out,int size){assert(size==16);for(int i=0;i<size;++i)out[i]=(unsigned char)(i*13);return 1;}
#ifdef REFERENCE
#include <dlfcn.h>
FILE *fopen64(const char *path,const char *access){assert(!strcmp(path,"/dev/urandom")&&!strcmp(access,"r"));FILE *f=tmpfile();assert(f);for(int i=0;i<17;++i)fputc(i*13,f);rewind(f);return f;}
#endif
int main(void){
#ifdef REFERENCE
 assert(dlopen("libnvram.so",RTLD_LAZY|RTLD_GLOBAL));void *h=dlopen("libshared.so",RTLD_NOW);assert(h);
 int (*init)(void)=dlsym(h,"init_enc_nvram");
 int (*enc)(char *,char *,char *)=dlsym(h,"enc_nvram"),(*dec)(char *,char *,char *)=dlsym(h,"dec_nvram");assert(init&&enc&&dec);
#else
 int (*init)(void)=init_enc_nvram;
 int (*enc)(char *,char *,char *)=enc_nvram,(*dec)(char *,char *,char *)=dec_nvram;
#endif
 char encoded[4096],decoded[4096],snapshot[7][4096];
 for(unsigned v=2;v<=3;++v)for(unsigned salted=0;salted<2;++salted)for(unsigned empty=0;empty<2;++empty){
  memset(values,0,sizeof values);memset(present,0,sizeof present);commits=0;version=v==2?"2":"3";
  if(salted)nvram_set("nvram_salt","synthetic-salt");
  char *password=empty?"":"example-password",*list=empty?"":"<alice>first-password<bob>second-password";
  assert(enc("http_passwd",password,encoded)>=0);nvram_set("http_passwd",encoded);
  assert(enc("acc_list",list,encoded)>=0);nvram_set("acc_list",encoded);
  assert(enc("former_secret","old-password",encoded)==1);nvram_set("former_secret",encoded);
  nvram_set("ordinary","short");
  for(unsigned pass=0;pass<2;++pass){
   assert(!init()&&commits==pass+1);
   assert(dec("http_passwd",values[0],decoded)==(empty?0:1)&&!strcmp(decoded,password));
   assert(dec("acc_list",values[1],decoded)==(empty?0:1)&&!strcmp(decoded,list));
   assert(!strcmp(values[2],"old-password")&&!strcmp(values[3],"short"));
   assert(!strcmp(values[5],"1")&&!strcmp(values[6],version));
   if(v==3)assert(present[4]);else assert(present[4]==salted);
   if(pass)assert(!memcmp(snapshot,values,sizeof values));else memcpy(snapshot,values,sizeof values);
   printf("C %u %u %u %u\n",v,salted,empty,pass);
   for(unsigned i=0;i<7;++i)printf("%s %u %s\n",names[i],present[i],values[i]);
  }
 }
 return 0;
}
