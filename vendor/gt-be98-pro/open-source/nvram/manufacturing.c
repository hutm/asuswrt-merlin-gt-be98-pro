/* SPDX-License-Identifier: GPL-2.0-only */
#define _POSIX_C_SOURCE 200809L
#include "manufacturing.h"
#include <unistd.h>
#include <errno.h>
#include <string.h>
int nv_mfg_append(FILE *input,FILE *output,FILE *base_mac) {
    char record[1024];size_t used=0;int c,has_mac=0;
    if(!input || !output) {errno=EINVAL;return -1;}
    for(;;) {
        c=fgetc(input);
        if(c==EOF) {
            if(ferror(input)) return -1;
            if(used) {errno=EPROTO;return -1;}
            break;
        }
        if(c) {
            if(used==sizeof record-1) {errno=EOVERFLOW;return -1;}
            record[used++]=(char)c;continue;
        }
        if(!used) break;
        record[used]=0;
        if(!strncmp(record,"et0macaddr=",11)) has_mac=1;
        if(fwrite(record,1,used,output)!=used || fputc('\n',output)==EOF) return -1;
        used=0;
    }
    if(!has_mac && base_mac) {
        char mac[19];size_t size=fread(mac,1,18,base_mac);
        if(ferror(base_mac)) return -1;
        mac[size]=0;
        if(size && fprintf(output,"et0macaddr=%s\n",mac)<0) return -1;
    }
    return ferror(output)?-1:0;
}

int nv_mfg_restore(const char *source,const char *destination,const char *base_mac) {
    FILE *input,*output,*mac=NULL;int result,saved;
    if(!source || !destination) {errno=EINVAL;return -1;}
    input=fopen(source,"rb");if(!input) return -1;
    output=fopen(destination,"a");
    if(!output) {saved=errno;fclose(input);errno=saved;return -1;}
    if(base_mac) mac=fopen(base_mac,"rb"); /* Optional, as in the reference. */
    result=nv_mfg_append(input,output,mac);saved=errno;
    if(!result && (fflush(output) || fsync(fileno(output)))) {result=-1;saved=errno;}
    if(mac && fclose(mac) && !result) {result=-1;saved=errno;}
    if(fclose(input) && !result) {result=-1;saved=errno;}
    if(fclose(output) && !result) {result=-1;saved=errno;}
    errno=saved;return result;
}
