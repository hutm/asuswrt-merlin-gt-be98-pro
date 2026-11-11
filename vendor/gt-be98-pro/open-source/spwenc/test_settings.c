/* SPDX-License-Identifier: GPL-2.0-only */
#include "settings.h"
#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
static int denied,wrong;
char *nvram_get(const char *name){assert(!strcmp(name,"productid"));return "TEST-router";}
char *nvram_get_salt(void){return denied?(char *)(intptr_t)-1:wrong?"different":"synthetic-salt";}
char *get_lan_hwaddr(void){return "02:11:22:33:44:55";}
#ifdef REFERENCE
#include <dlfcn.h>
#endif
int main(void){
#ifdef REFERENCE
 assert(dlopen("libnvram.so",RTLD_LAZY|RTLD_GLOBAL));void *h=dlopen("libshared.so",RTLD_NOW);assert(h);
 int (*enc)(char *,char *,char *)=dlsym(h,"enc_nvram");assert(enc);
 int (*dec)(char *,char *,char *)=dlsym(h,"dec_nvram");assert(dec);
#else
 int (*enc)(char *,char *,char *)=enc_nvram;
 int (*dec)(char *,char *,char *)=dec_nvram;
#endif
 char *names[]={"acc_list","pptpd_clientlist","vpn_serverx_clientlist","http_passwd","custom"};
 char *values[]={"","broken","<a>pass","<a>pass<b>next","a><>pass","a>pass>extra<b>next","<<a>pass<<b>next<",">","a>>pass"};
 char out[4096],decoded[4096];
 for(unsigned n=0;n<5;++n)for(unsigned v=0;v<9;++v){
  strcpy(out,"unchanged");int ret=enc(names[n],values[v],out);assert(ret==(v?1:0));printf("%u %u %d %s\n",n,v,ret,out);
  for(wrong=0;wrong<2;++wrong){int r=dec(names[n],out,decoded);printf("D %u %u %d %d %s\n",n,v,wrong,r,decoded);}
  wrong=0;
 }
 char input[513];
 for(unsigned len=1;len<=512;++len){memset(input,'a'+len%26,len);input[len]=0;assert(enc(names[3],input,out)==1);printf("L %u %s\n",len,out);assert(dec(names[3],out,decoded)==1&&!strcmp(input,decoded));}
/* Last usable record determines the list return status. */
 assert(enc(names[3],"pass",out)==1);
 char mixed[4096];
 for(unsigned order=0;order<2;++order){
  snprintf(mixed,sizeof mixed,order?"<a><b>%.4000s":"<b>%.4000s<a>",out);
  int r=dec(names[0],mixed,decoded);assert(r==(int)order);printf("M %u %d %s\n",order,r,decoded);
 }
#ifndef REFERENCE
 strcpy(out,"unchanged");
 assert(nv_enc_setting("acc_list","<a>pass",out,1)==-1&&errno==ENOSPC&&!strcmp(out,"unchanged"));
 denied=1;assert(dec(names[0],mixed,out)==-1&&errno==EACCES&&!strcmp(out,"unchanged"));assert(enc(names[0],values[2],out)==-1&&errno==EACCES&&!strcmp(out,"unchanged"));denied=0;
 char oversized[4097];memset(oversized,'a',sizeof oversized-1);oversized[4096]=0;
 assert(enc(names[3],oversized,out)==-1&&errno==ENOSPC&&!strcmp(out,"unchanged"));
 oversized[4000]='>';oversized[4001]='a';oversized[4002]=0;
 assert(enc(names[0],oversized,out)==1);
 oversized[4090]='>';oversized[4091]='a';oversized[4092]=0;oversized[4000]='a';oversized[4002]='a';
 assert(enc(names[0],oversized,out)==-1&&errno==ENOSPC);
 assert(nv_dec_setting(names[0],mixed,out,1)==-1&&errno==ENOSPC);
 assert(nv_dec_setting(NULL,"x",out,sizeof out)==-1&&errno==EINVAL);
 assert(nv_enc_setting(NULL,"x",out,sizeof out)==-1&&errno==EINVAL);
#endif
 return 0;
}
