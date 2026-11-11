/* SPDX-License-Identifier: GPL-2.0-only */
#include "policy.h"
#include <limits.h>
#include <stdlib.h>
#include <string.h>
extern int webapi_get_b(int,char *,size_t);
extern char *nvram_get(const char *);
extern int nvram_get_int(const char *);
struct ASUS_PP_table ASUS_PP_t[]={
 {"AUTOUPGRADE","1",1,0},{"ASD","1",2,0},{"AHS","1",3,0},
 {"ACCOUNT_BINDING","2",4,1},{"CONFIG_TRANSFER","2",5,1},
 {"ASUS_DDNS","2",6,1},{"ASUS_AFC","4",7,0},
 {"LICENSE_UPDATE_V5","5",8,1},{NULL,NULL,0,0}
};
/* Match the platform's 32-bit strtol range on native test hosts too. */
static int number(const char *s)
{
 long value=strtol(s,NULL,10);
 if(value>INT_MAX)return INT_MAX;
 if(value<INT_MIN)return INT_MIN;
 return (int)value;
}
int get_ASUS_privacy_policy_ver(int id)
{
 for(struct ASUS_PP_table *t=ASUS_PP_t;t->name;++t)
  if(t->id==id)return number(t->version);
 return 0;
}
int get_ASUS_privacy_policy(void)
{
 char name[32]={0};
 if(webapi_get_b(3,name,sizeof name)<=0)return 0;
 name[sizeof name-1]=0;
 return nvram_get_int(name);
}
int get_ASUS_privacy_policy_state(int id)
{
 char name[32]={0},value[32]={0};
 if(webapi_get_b(3,name,sizeof name)<=0)return 0;
 name[sizeof name-1]=0;
 const char *stored=nvram_get(name);
 if(stored){size_t n=strlen(stored);if(n>=sizeof value)n=sizeof value-1;memcpy(value,stored,n);}
 int required=get_ASUS_privacy_policy_ver(id),actual=number(value);
 return actual!=0 && required!=0 && actual>=required;
}
