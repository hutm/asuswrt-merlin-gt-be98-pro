/* SPDX-License-Identifier: GPL-2.0-only */
#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
extern char *get_auth_code(char *,char *,int);
int main(void){
 int caps[]={INT_MIN,-1,0,1,63,64,514,INT_MAX};
 for(unsigned i=0;i<sizeof(caps)/sizeof(caps[0]);i++){
  char out[80],before[80];memset(out,0x5a,sizeof(out));memcpy(before,out,sizeof(out));errno=0;
  assert(get_auth_code("1|test",out,caps[i])==0);assert(errno==EINVAL);assert(!memcmp(before,out,sizeof(out)));
 }
 errno=0;assert(!get_auth_code("1|test",0,65));assert(errno==EINVAL);return 0;
}
