/* SPDX-License-Identifier: GPL-2.0-only */
#include <errno.h>
#include <stdio.h>
#include <string.h>
char *wlcsm_trim_str(char *value) {
    size_t length;
    if(!value) return NULL;
    length=strlen(value);
    while(length && (value[length-1]==' ' || value[length-1]=='"' || value[length-1]=='\n')) --length;
    value[length]=0;
    while(*value==' ' || *value=='"') ++value;
    return value;
}
char *wlcsm_trim_str2(char *value) {
    size_t length;
    if(!value) return NULL;
    length=strlen(value);
    while(length && value[length-1]=='\n') --length;
    value[length]=0;
    return value;
}
char *wlcsm_nvram_name_parser(char *name,int *radio,int *subunit) {
    char *separator;int count;
    if(!radio || !subunit) {errno=EINVAL;return NULL;}
    if(name && (separator=strchr(name,'_'))) {
        count=sscanf(name,"wl%d.%d",radio,subunit);
        if(count>0) {if(count==1) *subunit=0;return separator+1;}
    }
    *radio=*subunit=0;return NULL;
}
