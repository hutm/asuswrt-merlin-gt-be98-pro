/* SPDX-License-Identifier: GPL-2.0-only */
#define _DEFAULT_SOURCE
#include "wrapper.h"
#include "inputs.h"
#include "password.h"
#include "lengths.h"
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <openssl/crypto.h>
extern char *nvram_get(const char *);
extern char *nvram_get_salt(void);
extern char *get_lan_hwaddr(void);
static const char *salt(void){return nvram_get_salt();}
static const char *product(void){return nvram_get("productid");}
static const char *mac(void){return get_lan_hwaddr();}
static void delay(unsigned usec){(void)usleep(usec);}
static const struct nv_pw_input_ops ops={salt,product,mac,delay};
int pw_enc(const char *input,char *output,int with_salt)
{
 struct nv_pw_inputs values;
 if(!input || !output){errno=EINVAL;return -1;}
 if(!*input){*output=0;return 0;}
 int size=pw_enc_blen(input);
 if(size<0 || nv_pw_inputs_load(&ops,with_salt,&values)<0)return -1;
 int result=nv_pw_encrypt(input,values.salt,values.product,values.mac,output,(size_t)size+1);
 OPENSSL_cleanse(&values,sizeof values);
 return result;
}
int pw_dec(const char *input,char *output,int len,int with_salt)
{
 struct nv_pw_inputs values;
 if(!input || !output || len<=0){errno=EINVAL;return -1;}
 if(!*input){*output=0;return 0;}
 if(nv_pw_inputs_load(&ops,with_salt,&values)<0)return -1;
 int result=nv_pw_decrypt(input,values.salt,values.product,values.mac,output,(size_t)len);
 int error=errno;
 OPENSSL_cleanse(&values,sizeof values);
 if(!result)return 1;
 if(error!=EPROTO && error!=EBADMSG){errno=error;return -1;}
 size_t count=strlen(input);
 if(count>=(size_t)len)count=(size_t)len-1;
 memmove(output,input,count);output[count]=0;
 return 0;
}
