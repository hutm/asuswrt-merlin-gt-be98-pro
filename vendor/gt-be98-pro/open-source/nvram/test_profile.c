/* SPDX-License-Identifier: GPL-2.0-only */
#include "profile.h"
#include <assert.h>
#include <errno.h>
#include <string.h>
static int calls,fail;static char seen[128];
static int apply(void *ctx,const char *key,const char *value){(void)ctx;++calls;snprintf(seen,sizeof seen,"%s=%s",key,value?value:"<unset>");if(fail){errno=EIO;return -1;}return 0;}
static FILE *input(int encoded,const char *payload,size_t length,size_t declared){FILE *f=tmpfile();assert(f);unsigned char h[8]={0};memcpy(h,encoded?"HDR2":"HDR1",4);h[4]=declared;h[5]=declared>>8;h[6]=declared>>16;h[7]=encoded?7:declared>>24;fwrite(h,1,8,f);for(size_t i=0;i<length;++i){unsigned char c=payload[i];if(encoded)c=c?(unsigned char)(0xff-c+7):0xfe;fputc(c,f);}rewind(f);return f;}
int main(void){
 const char data[]="lan_ipaddr=192.0.2.1\0empty=\0remove\0\0";
 for(int encoded=0;encoded<2;++encoded){FILE *f=input(encoded,data,sizeof data,sizeof data);calls=0;assert(!nv_profile_read(f,262144,apply,NULL)&&calls==3&&!strcmp(seen,"remove=<unset>"));fclose(f);}
 FILE *f=input(0,data,1,262145);calls=0;assert(nv_profile_read(f,262144,apply,NULL)==-1&&errno==EOVERFLOW&&!calls);fclose(f);
 f=input(1,data,1,20);assert(nv_profile_read(f,262144,apply,NULL)==-1&&errno==EPROTO&&!calls);fclose(f);
 const char bad[]="valid=yes\0unterminated";f=input(0,bad,sizeof bad-1,sizeof bad-1);assert(nv_profile_read(f,262144,apply,NULL)==-1&&errno==EPROTO&&!calls);fclose(f);
 f=input(0,data,sizeof data,sizeof data);fail=1;assert(nv_profile_read(f,262144,apply,NULL)==-1&&errno==EIO&&calls==1);fclose(f);
 puts("Profile reader: HDR1/HDR2, bounded lengths, complete validation and callback failure passed");return 0;
}
