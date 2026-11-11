/* SPDX-License-Identifier: GPL-2.0-only */
#define _DEFAULT_SOURCE
#include "settings.h"
#include "wrapper.h"
#include "lengths.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/crypto.h>
static int encrypt(const char *input,char output[4096])
{
 int size=pw_enc_blen(input);
 if(size<0)return -1;
 if(size>=4096){errno=ENOSPC;return -1;}
 return pw_enc(input,output,1);
}
int nv_enc_setting(const char *name,const char *input,char *output,size_t capacity)
{
 char result[4096]={0},encoded[4096]={0};char *copy=NULL;size_t copy_size=0,used=0;
 int status=-1;
 if(!name||!input||!output){errno=EINVAL;return -1;}
 if(!capacity){errno=ENOSPC;return -1;}
 if(!*input){output[0]=0;return 0;}
 if(strcmp(name,"acc_list")&&strcmp(name,"pptpd_clientlist")&&strcmp(name,"vpn_serverx_clientlist")){
  if(encrypt(input,result)<0)goto done;
 }else{
  copy=strdup(input);if(!copy)goto done;copy_size=strlen(copy)+1;
  char *cursor=copy,*record;
  while((record=strsep(&cursor,"<"))){
   char *user=strsep(&record,">"),*password=strsep(&record,">");
   if(!password)continue;
   if(encrypt(password,encoded)<0)goto done;
   size_t user_len=strlen(user),encoded_len=strlen(encoded);
   if(user_len>4093 || encoded_len>4093-user_len || used>4093-user_len-encoded_len){errno=ENOSPC;goto done;}
   size_t prefix=user_len+encoded_len+2;
   memmove(result+prefix,result,used+1);
   result[0]='<';memcpy(result+1,user,user_len);result[user_len+1]='>';
   memcpy(result+user_len+2,encoded,encoded_len);used+=prefix;
  }
 }
 if(strlen(result)>=capacity){errno=ENOSPC;goto done;}
 memcpy(output,result,strlen(result)+1);status=1;
done:
 {int error=errno;OPENSSL_cleanse(result,sizeof result);OPENSSL_cleanse(encoded,sizeof encoded);
 OPENSSL_clear_free(copy,copy_size);errno=error;}
 return status;
}
int enc_nvram(char *name,char *input,char *output)
{
 return nv_enc_setting(name,input,output,4096);
}
