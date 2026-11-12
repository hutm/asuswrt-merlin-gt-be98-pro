/* SPDX-License-Identifier: GPL-2.0-only */
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
char *ATE_FACTORY_MODE_STR(void);
char *ATE_UPGRADE_MODE_STR(void);
int ate_brcm_factory_mode(void);
static const char *value;
static unsigned int calls;
static int fail_alloc;
char *__real_strdup(const char *);
char *__wrap_strdup(const char *s){if(fail_alloc){errno=ENOMEM;return NULL;}return __real_strdup(s);}
char *nvram_get(const char *key){assert(!strcmp(key,"ATEMODE"));calls++;return (char *)value;}
static void check(const char *v){value=v;calls=0;int result=ate_brcm_factory_mode();assert(result==(v&&!strcmp(v,"1")));assert(calls==1);printf("%d\n",result);}
int main(void){
 char *p=ATE_FACTORY_MODE_STR();assert(!strcmp(p,"ATEMODE"));
 for(int i=0;i<8;i++){memset(p,'x',8);assert(ATE_FACTORY_MODE_STR()==p);assert(!strcmp(p,"ATEMODE"));}
 char *a=ATE_UPGRADE_MODE_STR(),*b=ATE_UPGRADE_MODE_STR();assert(a&&b&&a!=b);assert(!strcmp(a,"ateUpgrade_flag")&&!strcmp(b,a));a[0]='X';assert(!strcmp(b,"ateUpgrade_flag"));free(a);free(b);
 fail_alloc=1;assert(!ATE_UPGRADE_MODE_STR()&&errno==ENOMEM);fail_alloc=0;
 check(NULL);check("");check("1");check("01");check(" 1");check("1 ");check("true");check("-1");
 for(int i=1;i<=255;i++){char s[3]={(char)i,0,0};check(s);s[1]='1';check(s);s[0]='1';s[1]=(char)i;check(s);}
 memset(p,'x',8);check("1");assert(!strcmp(p,"ATEMODE"));
 puts("774 settings cases; string ownership, reset and allocation failure checks passed");return 0;
}
