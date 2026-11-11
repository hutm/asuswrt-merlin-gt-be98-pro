/* SPDX-License-Identifier: GPL-2.0-only */
#include "cli_kernel.h"
#include "runtime.h"
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
static int number(const char *text,size_t size,unsigned limit,unsigned *out) {
    unsigned value=0;if(!size) {errno=EINVAL;return -1;}
    for(size_t i=0;i<size;++i) {
        unsigned digit=(unsigned char)text[i]-'0';
        if(digit>9 || digit>limit || value>(limit-digit)/10) {errno=EINVAL;return -1;}
        value=value*10+digit;
    }
    *out=value;return 0;
}
int nv_cli_flags(int argc,char *const argv[],FILE *output) {
    const char *separator,*value;unsigned bit,enabled;
    if(argc<=0 || !argv || !argv[0] || !output) {errno=EINVAL;return -1;}
    if(strcmp(argv[0],"getflag") && strcmp(argv[0],"setflag")) return 0;
    if(argc<3 || !argv[1] || !argv[2]) {errno=EINVAL;return -1;}
    if(!strcmp(argv[0],"getflag")) {
        if(number(argv[2],strlen(argv[2]),31,&bit)) return -1;
        value=wlcsm_nvram_get_bitflag(argv[1],bit);
        if(value && fprintf(output,"%s\n",value)<0) return -1;
        return 3;
    }
    separator=strchr(argv[2],'=');
    if(!separator || number(argv[2],(size_t)(separator-argv[2]),31,&bit) ||
       number(separator+1,strlen(separator+1),INT_MAX,&enabled)) {errno=EINVAL;return -1;}
    value=wlcsm_nvram_get(argv[1]);
    if(fprintf(output,"value:%s->",value?value:"NULL")<0) return -1;
    if(wlcsm_nvram_set_bitflag(argv[1],bit,(int)enabled)<0) return -1;
    value=wlcsm_nvram_get(argv[1]);
    if(value && fprintf(output,"%s\n",value)<0) return -1;
    return 3;
}
