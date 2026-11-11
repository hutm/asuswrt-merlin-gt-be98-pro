/* SPDX-License-Identifier: GPL-2.0-only */
#include "frame.h"
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
int main(void){
 char input[65];unsigned char output[96];size_t written;
 for(int n=0;n<65;++n){
  memset(input,'a',n);input[n]=0;memset(output,0xa5,sizeof output);
  assert(!nv_pw_frame(input,output,sizeof output,&written));
  if(!n){assert(written==0&&output[0]==0xa5);continue;}
  assert(written==(size_t)((n+23)&~15));assert(output[written]==0xa5);
  char decoded[65];assert(!nv_pw_unframe(output,written,decoded,sizeof decoded)&&!strcmp(decoded,input));
  printf("FRAME %d ",n+8);for(size_t i=0;i<written;++i)printf("%02x",output[i]);puts("");
 }
 memset(output,0xa5,sizeof output);written=9;
 assert(nv_pw_frame("a",output,15,&written)==-1&&errno==ENOSPC&&!written);
 for(unsigned i=0;i<sizeof output;++i)assert(output[i]==0xa5);
 assert(nv_pw_frame(NULL,output,sizeof output,&written)==-1&&errno==EINVAL);
 assert(nv_pw_frame("a",NULL,16,&written)==-1&&errno==EINVAL);
 assert(nv_pw_frame("a",output,16,NULL)==-1&&errno==EINVAL);
 char decoded[65];strcpy(decoded,"unchanged");
 assert(!nv_pw_frame("a",output,sizeof output,&written));
 assert(nv_pw_unframe(output,written,decoded,1)==-1&&errno==ENOSPC&&!strcmp(decoded,"unchanged"));
 output[0]^=1;assert(nv_pw_unframe(output,written,decoded,sizeof decoded)==-1&&errno==EBADMSG&&!strcmp(decoded,"unchanged"));output[0]^=1;
 output[15]=1;assert(nv_pw_unframe(output,written,decoded,sizeof decoded)==-1&&errno==EPROTO&&!strcmp(decoded,"unchanged"));output[15]=0;
 assert(nv_pw_unframe(output,15,decoded,sizeof decoded)==-1&&errno==EPROTO);
 assert(nv_pw_unframe(output,32,decoded,sizeof decoded)==-1&&errno==EPROTO);
 assert(nv_pw_unframe(NULL,16,decoded,sizeof decoded)==-1&&errno==EINVAL);
 assert(!nv_pw_unframe(NULL,0,decoded,sizeof decoded)&&!decoded[0]);
 return 0;
}
