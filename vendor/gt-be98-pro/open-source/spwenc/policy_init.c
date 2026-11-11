/* SPDX-License-Identifier: GPL-2.0-only */
#include "policy.h"
#include "names.h"
#include <stdio.h>
#include <string.h>
extern char *nvram_get(const char *);
extern int nvram_get_int(const char *);
extern int nvram_set(const char *,const char *);
static void snapshot(const char *name,char output[1024])
{
 const char *value=nvram_get(name);size_t n=value?strlen(value):0;
 if(n>1023)n=1023;
 if(n)memcpy(output,value,n);
 output[n]=0;
}
static void normalize(const char *name,const char *value,int version)
{
 if(!*value)return;
 for(const unsigned char *p=(const unsigned char *)value;*p;++p)
  if(*p<'0'||*p>'9')return;
 char decimal[32],record[1024];
 snprintf(decimal,sizeof decimal,"%d",version);decimal[7]=0;
 size_t prefix=strlen(decimal)+2;
 record[0]='<';memcpy(record+1,decimal,prefix-2);record[prefix-1]='>';
 size_t n=strlen(value);if(n>1023-prefix)n=1023-prefix;
 memcpy(record+prefix,value,n);record[prefix+n]=0;
 nvram_set(name,record);
}
void init_asus_pp_eula(void)
{
 char value[1024],name[32]={0};
 snapshot("ASUS_NEW_EULA_ts",value);
 int version=nvram_get_int("ASUS_NEW_EULA");
 normalize("ASUS_NEW_EULA_ts",value,version?version:1);
 /* Preserve lookup-failure behavior: reuse the previous version/value when
  * their replacement cannot be obtained. The normal fixed mapping succeeds. */
 if(webapi_get_b(3,name,sizeof name)>0)version=nvram_get_int(name);
 else if(!version)version=1;
 if(webapi_get_b(5,name,sizeof name)>0)snapshot(name,value);
 normalize(name,value,version);
}
