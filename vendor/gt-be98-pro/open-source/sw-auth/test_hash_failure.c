/* SPDX-License-Identifier: GPL-2.0-only */
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <openssl/sha.h>
extern char *get_auth_code(char *,char *,int);
extern unsigned char *__real_SHA256(const unsigned char *,size_t,unsigned char *);
static unsigned calls,fail_at;
unsigned char *__wrap_SHA256(const unsigned char *s,size_t n,unsigned char *out){
 if(++calls==fail_at)return NULL;
 return __real_SHA256(s,n,out);
}
int main(void){
 for(fail_at=1;fail_at<=21;fail_at++){
  char out[65];memset(out,0x5a,sizeof(out));calls=0;errno=0;
  assert(!get_auth_code("1|example",out,sizeof(out)));assert(errno==EIO);assert(calls==fail_at);
  for(unsigned i=0;i<sizeof(out);i++)assert(out[i]==0);
 }
 return 0;
}
