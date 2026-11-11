/* SPDX-License-Identifier: GPL-2.0-only */
#include "manufacturing.h"
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
