/* SPDX-License-Identifier: GPL-2.0-only */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <typedefs.h>
#include <bcmwifi_channels.h>
int main(void){
 for(unsigned int i=0;i<=65535;i++){
  chanspec_t c=(chanspec_t)i;int malformed=wf_chspec_malformed(c),valid=wf_chspec_valid(c);char text[128];memset(text,0,sizeof(text));
  printf("%u %d %d",i,malformed,valid);
  if(valid){assert(!malformed);char *p=wf_chspec_ntoa(c,text);assert(p);printf(" %s %u %u",p,(unsigned)wf_chspec_primary20_chan(c),(unsigned)wf_chspec_primary20_chspec(c));}
  putchar('\n');
 }
 return 0;
}
