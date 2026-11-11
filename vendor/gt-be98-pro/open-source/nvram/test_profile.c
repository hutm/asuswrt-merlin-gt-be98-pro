/* SPDX-License-Identifier: GPL-2.0-only */
#include "profile.h"
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
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
 fail=0;char settings[4096]={0};memcpy(settings,data,sizeof data);
 for(unsigned salt=0;salt<30;++salt){
  f=tmpfile();assert(f);calls=0;assert(!nv_profile_write(f,settings,sizeof settings,salt));assert(ftell(f)==1032);
  rewind(f);unsigned char header[8];assert(fread(header,1,8,f)==8);assert(!memcmp(header,"HDR2",4)&&header[4]==0&&header[5]==4&&header[6]==0&&header[7]==salt);
  rewind(f);assert(!nv_profile_read(f,sizeof settings,apply,NULL)&&calls==3);fclose(f);assert(!memcmp(settings,data,sizeof data));
 }
 f=tmpfile();assert(f);assert(nv_profile_write(f,"unterminated",12,7)==-1&&errno==EPROTO&&ftell(f)==0);
 assert(nv_profile_write(f,settings,8,7)==-1&&ftell(f)==0);
 assert(nv_profile_write(f,settings,sizeof settings,30)==-1&&errno==EINVAL&&ftell(f)==0);
 settings[0]=7;settings[1]=0;settings[2]=0;assert(nv_profile_write(f,settings,sizeof settings,7)==-1&&errno==EILSEQ&&ftell(f)==0);fclose(f);
 memset(settings,'x',1023);settings[1023]=0;settings[1024]=0;
 f=tmpfile();assert(f);assert(!nv_profile_write(f,settings,sizeof settings,7)&&ftell(f)==2056);fclose(f);
 memset(settings,0,sizeof settings);f=tmpfile();assert(f);assert(!nv_profile_write(f,settings,sizeof settings,7)&&ftell(f)==1032);fclose(f);
 f=fopen("/dev/full","w");assert(f);assert(nv_profile_write(f,settings,sizeof settings,7)==-1&&errno==ENOSPC);fclose(f);
 puts("Profile writer: bounded encoding, all salts, input preservation, padding and output failure passed");
 puts("Profile reader: HDR1/HDR2, bounded lengths, complete validation and callback failure passed");return 0;
}
