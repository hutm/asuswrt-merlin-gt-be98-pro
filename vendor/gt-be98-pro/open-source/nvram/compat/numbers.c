/* SPDX-License-Identifier: GPL-2.0-only */
#include <errno.h>
#include <stdint.h>
#include <stdlib.h>
/* ARM compatibility: the result slot is 32 bits even in a native host test. */
int wlcsm_strtol(const char *text,int base,int32_t *output) {
    char *end=NULL;long long value;
    errno=0;
    if(!text) return 0;
    if(!output) {errno=EINVAL;return -1;}
    value=strtoll(text,&end,base);
    if(value>INT32_MAX) {value=INT32_MAX;errno=ERANGE;}
    if(value<INT32_MIN) {value=INT32_MIN;errno=ERANGE;}
    *output=(int32_t)value;
    if((errno==ERANGE && (value==INT32_MIN || value==INT32_MAX)) ||
       (errno && !value) || end==text) return -1;
    return 0;
}
