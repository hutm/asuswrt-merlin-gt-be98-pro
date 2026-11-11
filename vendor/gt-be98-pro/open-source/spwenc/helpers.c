/* SPDX-License-Identifier: GPL-2.0-only */
#define _DEFAULT_SOURCE
#include "helpers.h"
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/evp.h>
#include <openssl/crypto.h>
extern char *nvram_get(const char *name);
char *str_to_md5(const char *input,int length,char *output)
{
 unsigned char digest[EVP_MAX_MD_SIZE];unsigned size=0;
 static const char hex[]="0123456789abcdef";
 if(!input || !output || length<0){errno=EINVAL;return NULL;}
 if(!EVP_Digest(input,(size_t)length,digest,&size,EVP_md5(),NULL)||size!=16){errno=EIO;return NULL;}
 for(unsigned i=0;i<16;++i){output[2*i]=hex[digest[i]>>4];output[2*i+1]=hex[digest[i]&15];}
 output[32]=0;OPENSSL_cleanse(digest,sizeof digest);return output;
}
/* Like the existing two-output vstrsep call, ignore fields after the second. */
static char *fields(char *record,char **user)
{
 *user=strsep(&record,">");return strsep(&record,">");
}
int is_same_pass(const char *name,const char *user,const char *password)
{
 if(!name||!user||!password){errno=EINVAL;return 0;}
 const char *value=nvram_get(name);char *copy=strdup(value?value:"");
 if(!copy)return 0;
 size_t size=strlen(copy);char *cursor=copy,*record;int match=0;
 while((record=strsep(&cursor,"<"))){
  char *stored_user,*stored=fields(record,&stored_user),digest[33];
  if(!stored)continue;
  if(!strcmp(name,"pptpd_clientlist")||!strcmp(name,"vpn_serverx_clientlist")){
   if(strlen(stored)>INT_MAX){errno=EOVERFLOW;break;}
   if(!str_to_md5(stored,(int)strlen(stored),digest))break;
   stored=digest;
  }
  match=!strcmp(user,stored_user)&&!strcmp(password,stored);
  OPENSSL_cleanse(digest,sizeof digest);
  if(match)break;
 }
 OPENSSL_clear_free(copy,size+1);return match;
}
void get_encrypt_list_value(const char *name,const char *user,char *output,int capacity)
{
 if(!name||!user||!output||capacity<0){errno=EINVAL;return;}
 const char *value=nvram_get(name);
 if(!value||!*value)return;
 char *copy=strdup(value);if(!copy)return;
 size_t size=strlen(copy);char *cursor=copy,*record;
 while((record=strsep(&cursor,"<"))){
  char *stored_user,*stored=fields(record,&stored_user);
  if(!stored||strcmp(user,stored_user))continue;
  if(capacity){size_t n=strlen(stored);if(n>=(size_t)capacity)n=(size_t)capacity-1;memcpy(output,stored,n);output[n]=0;}
  break;
 }
 OPENSSL_clear_free(copy,size+1);
}
