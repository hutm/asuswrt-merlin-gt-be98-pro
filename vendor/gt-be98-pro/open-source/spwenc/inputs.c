/* SPDX-License-Identifier: GPL-2.0-only */
#include "inputs.h"
#include <errno.h>
#include <stdint.h>
#include <string.h>
static void copy(char *out, unsigned size, const char *value)
{
 unsigned i=0;
 if(value) while(i+1<size && value[i]) { out[i]=value[i]; ++i; }
 out[i]=0;
}
static void retry(char *out, unsigned size, const char *(*get)(void),
                  void (*delay)(unsigned))
{
 for(unsigned i=0;i<3;++i) {
  copy(out,size,get());
  if(out[0] || i==2) break;
  delay(500);
 }
}
int nv_pw_inputs_load(const struct nv_pw_input_ops *ops, int with_salt,
                      struct nv_pw_inputs *out)
{
 struct nv_pw_inputs value={0};
 if(!ops || !out || !ops->product || !ops->mac || !ops->delay ||
    (with_salt && !ops->salt)) { errno=EINVAL; return -1; }
 if(with_salt) {
  const char *salt=ops->salt();
  if(salt==(const char *)(intptr_t)-1) { errno=EACCES; return -1; }
  copy(value.salt,sizeof value.salt,salt);
 }
 retry(value.product,sizeof value.product,ops->product,ops->delay);
 retry(value.mac,sizeof value.mac,ops->mac,ops->delay);
 if(!value.mac[0]) memcpy(value.mac,"00:00:00:00:00:00",18);
 *out=value;
 return 0;
}
