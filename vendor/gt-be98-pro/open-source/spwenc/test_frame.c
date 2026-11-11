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
  printf("FRAME %d ",n+8);for(size_t i=0;i<written;++i)printf("%02x",output[i]);puts("");
 }
 memset(output,0xa5,sizeof output);written=9;
 assert(nv_pw_frame("a",output,15,&written)==-1&&errno==ENOSPC&&!written);
 for(unsigned i=0;i<sizeof output;++i)assert(output[i]==0xa5);
 assert(nv_pw_frame(NULL,output,sizeof output,&written)==-1&&errno==EINVAL);
 assert(nv_pw_frame("a",NULL,16,&written)==-1&&errno==EINVAL);
 assert(nv_pw_frame("a",output,16,NULL)==-1&&errno==EINVAL);
 return 0;
}
