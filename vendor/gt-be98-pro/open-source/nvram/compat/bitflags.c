/* SPDX-License-Identifier: GPL-2.0-only */
#include <ctype.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
extern char *wlcsm_nvram_get(const char *);
extern int wlcsm_nvram_set(const char *,const char *);
/* Preserve the ARM library's 32-bit unsigned-long parsing on native hosts too. */
static uint32_t parse_hex32(const char *value) {
    unsigned long long magnitude;int negative=0,saved=errno;
    while(isspace((unsigned char)*value)) ++value;
    if(*value=='-' || *value=='+') {negative=*value=='-';++value;
        if(*value=='-' || *value=='+' || isspace((unsigned char)*value)) return 0;
    }
    errno=0;magnitude=strtoull(value,NULL,16);
    if(errno==ERANGE || magnitude>UINT32_MAX) {errno=ERANGE;return UINT32_MAX;}
    errno=saved;
    return negative?UINT32_C(0)-(uint32_t)magnitude:(uint32_t)magnitude;
}
char *wlcsm_nvram_get_bitflag(const char *key,unsigned bit) {
    const char *value;
    if(bit>31) return NULL;
    value=wlcsm_nvram_get(key);if(!value) return NULL;
    return parse_hex32(value)&(UINT32_C(1)<<bit)?"1":"0";
}
int wlcsm_nvram_set_bitflag(const char *key,unsigned bit,int enabled) {
    const char *value;uint32_t bits=0;char output[16];
    if(bit>31) return 0;
    value=wlcsm_nvram_get(key);
    /* Existing missing-key behavior writes zero even when enabling a bit. */
    if(value) {
        bits=parse_hex32(value);
        if(enabled) bits|=UINT32_C(1)<<bit;else bits&=~(UINT32_C(1)<<bit);
    }
    snprintf(output,sizeof output,"%x",(unsigned)bits);
    return wlcsm_nvram_set(key,output);
}
char *nvram_get_bitflag(const char *key,unsigned bit) {return wlcsm_nvram_get_bitflag(key,bit);}
int nvram_set_bitflag(const char *key,unsigned bit,int enabled) {return wlcsm_nvram_set_bitflag(key,bit,enabled);}
