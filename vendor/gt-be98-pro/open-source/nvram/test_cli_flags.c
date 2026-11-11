/* SPDX-License-Identifier: GPL-2.0-only */
#include "cli_kernel.h"
#include <assert.h>
#include <errno.h>
#include <string.h>
static int calls,missing,fail;static const char *value="0";
char *wlcsm_nvram_get(const char *key){assert(!strcmp(key,"flags"));return missing?NULL:(char *)value;}
char *wlcsm_nvram_get_bitflag(const char *key,unsigned bit){assert(!strcmp(key,"flags")&&bit==3);++calls;return "1";}
int wlcsm_nvram_set_bitflag(const char *key,unsigned bit,int enabled){assert(!strcmp(key,"flags")&&bit==3&&enabled==1);++calls;if(fail){errno=EIO;return -1;}value="8";return 0;}
int main(void){FILE *out=tmpfile();assert(out);char *args[]={"getflag","flags","3"};assert(nv_cli_flags(3,args,out)==3&&calls==1);args[0]="setflag";args[2]="3=1";assert(nv_cli_flags(3,args,out)==3&&calls==2&&!strcmp(args[2],"3=1"));
 rewind(out);char text[64]={0};fread(text,1,sizeof text-1,out);assert(!strcmp(text,"1\nvalue:0->8\n"));fclose(out);out=tmpfile();assert(out);
 const char *bad[]={"", "32=1", "-1=1", "3=", "3=1x", "3=2147483648", "=1", "3"};for(unsigned i=0;i<sizeof bad/sizeof bad[0];++i){args[2]=(char *)bad[i];assert(nv_cli_flags(3,args,out)==-1&&errno==EINVAL&&calls==2);}
 args[2]="3=1";fail=1;assert(nv_cli_flags(3,args,out)==-1&&errno==EIO);fail=0;missing=1;assert(nv_cli_flags(3,args,out)==3);assert(nv_cli_flags(2,args,out)==-1);fclose(out);
 puts("CLI flags: argument bounds, backend routing, output, missing values and failures passed");return 0;}
