/* SPDX-License-Identifier: GPL-2.0-only */
/* Generate a profile for restore compatibility tests, without router access. */
#include "profile.h"
int main(int argc,char **argv) {
    const char settings[4096]="lan_ipaddr=192.0.2.1\0wl_test=value\0wl0_failed=skip\0wgc1_test=keep\0test_exact=keep\0unknown=skip\0\0";
    FILE *out;int result;
    if(argc!=2) return 2;
    out=fopen(argv[1],"wb");if(!out) return 1;
    result=nv_profile_write(out,settings,sizeof settings,7);
    if(fclose(out)) result=-1;
    return result?1:0;
}
