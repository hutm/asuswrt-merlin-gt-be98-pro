/* SPDX-License-Identifier: GPL-2.0-only */
/* Read-only reference calls; does not access settings or hardware. */
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int main(void) {
 void *runtime=dlopen("libnvram.so",RTLD_LAZY|RTLD_GLOBAL);if(!runtime){fprintf(stderr,"%s\n",dlerror());return 1;}
 void *h=dlopen("libshared.so",RTLD_NOW);if(!h){fprintf(stderr,"%s\n",dlerror());return 1;}
 int (*enc)(const char *)=dlsym(h,"pw_enc_len");
 int (*blen)(const char *)=dlsym(h,"pw_enc_blen");
 int (*dec)(const char *)=dlsym(h,"pw_dec_len");
 if(!enc||!blen||!dec)return 2;
 char *s=malloc(65537);if(!s)return 3;memset(s,'a',65536);s[65536]=0;
 for(int n=0;n<=65536;++n){char old=s[n];s[n]=0;printf("%d %d %d %d\n",n,enc(s),blen(s),dec(s));s[n]=old;}
 free(s);dlclose(h);return 0;
}
