/* SPDX-License-Identifier: GPL-2.0-only */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <errno.h>
#include <limits.h>
extern char *nvram_get(const char *);
extern int nvram_get_int(const char *),nvram_get_hex(const char *),nvram_set_int(const char *,int),nvram_set(const char *,const char *),nvram_commit(void),notify_rc(const char *),get_model(void);
struct defaults_entry {int model;char *odmpid,*territory,*name,*value;};
extern struct defaults_entry tcode_init_nvram_list[];
static const char *get(const char *name){const char *v=nvram_get(name);return v?v:"";}
static int32_t hex(const char *s){long long n=strtoll(s,NULL,16);if(n>INT32_MAX){errno=ERANGE;return INT32_MAX;}if(n<INT32_MIN){errno=ERANGE;return INT32_MIN;}return (int32_t)n;}
char *tcode_default_get(const char *name){
 char territory[7];if(snprintf(territory,sizeof(territory),"%s",get("territory_code"))<=0)return NULL;
 int model=get_model();const char *odm=get("odmpid");
 for(struct defaults_entry *e=tcode_init_nvram_list;e->model;e++){
  if(e->model==model&&(!e->odmpid||!strcmp(e->odmpid,odm))&&(!*e->territory||!strcmp(e->territory,territory))&&!strcmp(e->name,name))return e->value;
  if(e->model==-1&&(!*e->territory||!strncmp(e->territory,territory,2))&&!strcmp(e->name,name))return e->value;
 }
 return NULL;
}
int asus_ctrl_en(int cid){uint32_t flags=(uint32_t)nvram_get_hex("asusctrl_flags");unsigned shift=(unsigned)cid&255;return (flags>>(shift<32?shift:31))&1;}
int asus_ctrl_ignore(void){return !*get("asusctrl_flags");}
int setting_SG_mode_wps(void){
 if(!(hex(get("asusctrl_flags"))&64)||nvram_get_int("SG_mode"))return 0;
 nvram_set_int("SG_mode",1);nvram_set_int("wps_enable",0);nvram_set_int("wps_enable_x",0);nvram_commit();return 1;
}
int asus_ctrl_nv(char *flags,int notify){
 int32_t value=hex(flags);
 if(!nvram_get_int("SG_mode")&&(value&64)){
  nvram_set_int("wan_upnp_enable",0);nvram_set_int("wan0_upnp_enable",0);nvram_set_int("wan1_upnp_enable",0);
  nvram_set_int("webs_update_enable",1);nvram_set("webs_update_time","03:00");
  if(!asus_ctrl_en(6)){
   nvram_set("webs_SG_mode","1");if(notify)notify_rc("stop_upnp");
   nvram_set_int("SG_mode",1);nvram_set_int("wps_enable",0);nvram_set_int("wps_enable_x",0);
   if(notify)notify_rc("restart_wpsie");
  }
  nvram_commit();
 }
 return 0;
}
int asus_ctrl_nv_restore(void){asus_ctrl_nv((char *)get("asusctrl_flags"),0);return 0;}
static int prefix(const char *key,int eu){char value[16];snprintf(value,sizeof(value),"%s",get(key));return eu?(!strncmp(value,"EU",2)||!strncmp(value,"UK",2)||!strncmp(value,"IL",2)):(!strncmp(value,"CN",2)||!strncmp(value,"CT",2));}
int is_CN_sku(void){return prefix("territory_code",0);}
int is_CN_location(void){return prefix("location_code",0);}
int is_EU_sku(void){return prefix("territory_code",1);}
int noasusddns(void){
 if(strncmp(get("territory_code"),"CN",2))return 0;
 char copy[64];snprintf(copy,sizeof(copy),"%s",get("coordinate"));char *next=copy,*part=NULL;
 for(int i=0;i<3;i++){part=strsep(&next,",");if(!part)return 1;}
 return !strcasecmp(part,"CN");
}
