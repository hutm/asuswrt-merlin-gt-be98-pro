/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef OPEN_NVRAM_SETTINGS_H
#define OPEN_NVRAM_SETTINGS_H
#include <errno.h>
#include <stdio.h>
#include <string.h>
typedef int (*nv_setting_fn)(void *,const char *,const char *);
/* Match the observed loader's space/quote trimming, not generic isspace(). */
static inline char *nv_trim_key(char *key) {
    size_t length=strlen(key);
    while(length && (key[length-1]==' ' || key[length-1]=='"' || key[length-1]=='\n')) key[--length]=0;
    while(*key==' ' || *key=='"') ++key;
    return key;
}
static inline int nv_settings_load(FILE *input,nv_setting_fn apply,void *context) {
    char line[1024];
    if(!input || !apply) { errno=EINVAL;return -1; }
    while(fgets(line,sizeof line,input)) {
        size_t length=strlen(line);
        char *separator,*value;
        /* Fail instead of interpreting chunks of an oversized physical line as
         * separate settings. This intentionally tightens the legacy parser. */
        if(length==sizeof line-1 && line[length-1]!='\n') {
            int next=fgetc(input);
            if(next!=EOF) { errno=EOVERFLOW;return -1; }
            if(ferror(input)) return -1;
        }
        if(line[0]=='#') continue;
        separator=strchr(line,'=');if(!separator) continue;
        *separator=0;value=separator+1;
        length=strlen(value);
        while(length && value[length-1]=='\n') value[--length]=0;
        if(apply(context,nv_trim_key(line),value)) return -1;
    }
    return ferror(input)?-1:0;
}
static inline int nv_settings_write(FILE *output,const void *data,size_t size) {
    const char *bytes=data;
    size_t offset=0;
    if(!output || (!data && size)) { errno=EINVAL;return -1; }
    /* Validate all record boundaries before writing. */
    while(offset<size && bytes[offset]) {
        const char *end=memchr(bytes+offset,0,size-offset);
        if(!end) { errno=EPROTO;return -1; }
        offset=(size_t)(end-bytes)+1;
    }
    offset=0;
    while(offset<size && bytes[offset]) {
        size_t length=strlen(bytes+offset);
        if(fwrite(bytes+offset,1,length,output)!=length || fputc('\n',output)==EOF) return -1;
        offset+=length+1;
    }
    return 0;
}
#endif
