/* SPDX-License-Identifier: GPL-2.0-only */
#include "migration.h"
#include "settings.h"
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/crypto.h>
#include <openssl/rand.h>
extern struct nvram_tuple router_defaults[];
extern char *nvram_get(const char *);
extern char *nvram_default_get(const char *);
extern char *nvram_get_salt(void);
extern int nvram_set(const char *,const char *);
extern int nvram_set_int(const char *,int);
extern int nvram_commit(void);
static int set(const char *name,const char *value)
{if(nvram_set(name,value)){errno=EIO;return -1;}return 0;}
int init_enc_nvram(void)
{
 char decoded[4096];int result=-1;
 for(struct nvram_tuple *t=router_defaults;t->name;++t){
  if(t->enc==1)continue;
  const char *value=nvram_get(t->name);if(!value)continue;
  size_t length=strlen(value);if(length<24||length>4095)continue;
  int status=dec_nvram(t->name,(char *)value,decoded);
  if(status<0)goto done;
  if(status && set(t->name,decoded)<0)goto done;
 }
 const char *version=nvram_default_get("nvram_encrypt_ver");int ver=0;
 if(version){
  errno=0;long parsed=strtol(version,NULL,10);
  if(errno==ERANGE||parsed<INT_MIN||parsed>INT_MAX){errno=ERANGE;goto done;}
  ver=(int)parsed;
  if(nvram_set_int("nvram_encrypt_ver",ver)){errno=EIO;goto done;}
 }
 if(start_dec_nvram()<0)goto done;
 if(version && ver>2){
  const char *salt=nvram_get_salt();
  if(salt==(const char *)(intptr_t)-1){errno=EACCES;goto done;}
  if(!salt){
   unsigned char random[16];char encoded[33];static const char hex[]="0123456789abcdef";
   if(RAND_bytes(random,sizeof random)!=1){OPENSSL_cleanse(random,sizeof random);errno=EIO;goto done;}
   for(unsigned i=0;i<sizeof random;++i){encoded[2*i]=hex[random[i]>>4];encoded[2*i+1]=hex[random[i]&15];}encoded[32]=0;
   int status=set("nvram_salt",encoded);OPENSSL_cleanse(random,sizeof random);OPENSSL_cleanse(encoded,sizeof encoded);
   if(status<0)goto done;
  }
 }
 if(start_enc_nvram()<0)goto done;
 const char *mode=nvram_default_get("nvram_encrypt");
 if(mode && set("nvram_encrypt",mode)<0)goto done;
 if(nvram_commit()){errno=EIO;goto done;}
 result=0;
done:
 {int error=errno;OPENSSL_cleanse(decoded,sizeof decoded);errno=error;}
 return result;
}
