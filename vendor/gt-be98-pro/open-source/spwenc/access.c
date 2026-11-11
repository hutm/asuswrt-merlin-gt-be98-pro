/* SPDX-License-Identifier: GPL-2.0-only */
#define _POSIX_C_SOURCE 200809L
#include "access.h"
#include <string.h>
#include <unistd.h>
struct INVALID_NVRAM_GET_TABLE invalid_nvram_get_t[]={
 {"nvram_salt",NULL},{"http_passwd",NULL},{"http_passwd_app",NULL},
 {"shell_passwd",NULL},{"PM_SMTP_AUTH_PASS",NULL},{"acc_list",NULL},
 {"pptpd_clientlist",NULL},{"vpn_serverx_clientlist",NULL},
 {"chilli_uamsecret",NULL},{"cp_uamsecret",NULL},{"zebra_passwd",NULL},
 {"zebra_enpasswd",NULL},{"rip_passwd",NULL},{"forget_it",NULL},
 {"oauth_dm_user_ticket",NULL},{"oauth_dm_refresh_ticket",NULL},
 {"http_oauth_clientlist",NULL},{"6e3f7d37",NULL},{"fdbca74d",NULL},
 {"afc_active_ts",NULL},{NULL,NULL}
};
int invalid_nvram_get_name(char *name)
{
 if(!name)return 0;
 for(struct INVALID_NVRAM_GET_TABLE *t=invalid_nvram_get_t;t->name;++t)
  if(!strcmp(t->name,name))return 1;
 return 0;
}
int invalid_program_check(void)
{
 char path[128];ssize_t size=readlink("/proc/self/exe",path,sizeof path-1);
 /* Deny when caller identity cannot be determined or may be truncated. */
 if(size<0 || (size_t)size>=sizeof path-1)return 1;
 path[size]=0;
 return strncmp(path,"/usr/",5)&&strncmp(path,"/bin/",5)&&strncmp(path,"/sbin/",6)?1:0;
}
int invalid_nvram_get_program(char *name)
{
 if(!name)return 0;
 if(!strcmp(name,"nvram_salt"))return 1;
 return invalid_nvram_get_name(name)?invalid_program_check():0;
}
