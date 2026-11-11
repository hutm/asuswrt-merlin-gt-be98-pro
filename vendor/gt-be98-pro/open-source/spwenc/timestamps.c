/* SPDX-License-Identifier: GPL-2.0-only */
#include "timestamps.h"
#include "names.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern int nvram_get_int(const char *);
extern char *nvram_get(const char *);
extern int nvram_set(const char *,const char *);
extern char *rfctime(const time_t *,char *,int);
static int32_t signed_bits(uint32_t bits){int32_t value;memcpy(&value,&bits,sizeof value);return value;}
static int adjust(const char *name,time_t before,int difference,char formatted[100])
{
 char text[11]={0};const char *stored=nvram_get(name);
 if(stored){size_t n=strlen(stored);if(n>10)n=10;memcpy(text,stored,n);}
 /* Ten input characters fit in int64; emulate the target's unsigned-long
  * parser and subsequent signed 32-bit time comparison without C overflow. */
 long long parsed=strtoll(text,NULL,10);
 uint32_t bits=parsed>UINT32_MAX?UINT32_MAX:(uint32_t)parsed;
 int32_t old=signed_bits(bits);
 if(!old || old>=before)return 0;
 time_t adjusted=signed_bits(bits+(uint32_t)difference);
 char full[32];snprintf(full,sizeof full,"%ld",(long)adjusted);
 size_t n=strlen(full);if(n>10)n=10;memcpy(text,full,n);text[n]=0;
 memset(formatted,0,100);rfctime(&adjusted,formatted,100);formatted[99]=0;
 nvram_set(name,text);return 1;
}
void update_ntp_ts(time_t before,int difference)
{
 char formatted[100],name[32]={0};
 if(nvram_get_int("ntp_ready")==1)return;
 if(adjust("ASUS_NEW_EULA_ts",before,difference,formatted))nvram_set("ASUS_NEW_EULA_time",formatted);
 if(webapi_get_b(5,name,sizeof name)>0 && adjust(name,before,difference,formatted)){
  if(webapi_get_b(4,name,sizeof name)>0)nvram_set(name,formatted);
 }
}
