/* SPDX-License-Identifier: GPL-2.0-only */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sysdeps/amas/amas_dwb.h"
struct connect_param_mapping_s connect_param_mapping_list[]={
 {"ssid"},{"bss_enabled"},{"wpa_psk"},{"auth_mode_x"},{"crypto"},
 {"mbss"},{"closed"},{"wep_x"},{"key"},{"key1"},{"key2"},{"key3"},{"key4"},{"11be"},{NULL}
};
const char *sc_basic_param[]={"smart_connect_x","bsd_ifnames","bsd_bounce_detect",NULL};
const char *sc_detailed_param[]={"bsd_steering_policy","bsd_sta_select_policy","bsd_if_select_policy","bsd_if_qualify_policy",NULL};
extern char *nvram_get(const char *);
extern int nvram_get_int(const char *);
extern int nvram_set(const char *,const char *);
extern int nvram_set_int(const char *,int);
extern int nvram_unset(const char *);
extern int nvram_commit(void);
static const char *get(const char *key){const char *v=nvram_get(key);return v?v:"";}
/* The vendor interface snapshots 31 bytes and splits on ASCII space only. */
static unsigned interfaces(void){char list[32];snprintf(list,sizeof(list),"%s",get("wl_ifnames"));unsigned n=0;char *p=list;while(*p){p+=strspn(p," ");if(!*p)break;n++;p+=strcspn(p," ");}return n;}
/* Preserve the legacy attempted-restoration result even if storage fails. */
int cm_revert_wifi_parameters(void){
 int band=nvram_get("dwb_band")?nvram_get_int("dwb_band"):-1;
 const char *ssid=get("dwb_band_ori_ssid");if(band<0||!*ssid)return 0;
 char key[64];snprintf(key,sizeof(key),"wl%d_ssid",band);nvram_set(key,get("dwb_band_ori_ssid"));
 snprintf(key,sizeof(key),"wl%d_closed",band);nvram_set_int(key,nvram_get_int("dwb_band_ori_closed"));
 nvram_unset("dwb_band_ori_ssid");nvram_unset("dwb_band_ori_closed");nvram_commit();return 1;
}
void dwb_init_settings(void){
 int rule=*get("amas_dwb_rule")?atoi(get("amas_dwb_rule")):-1;
 unsigned count=interfaces();
 if(!nvram_get("dwb_band")||!*get("dwb_band"))nvram_set_int("dwb_band",rule==0?(int)count:2);
 int re=nvram_get_int("re_mode");int subunit=nvram_get_int(re==1?"mlo_re_mssid_subunit":"mlo_cap_mssid_subunit");
 nvram_set_int("mlo_dwb_mssid_subunit",subunit);
}
/* This model ignores bandNum; dwb_band chooses the excluded interface. */
void cm_revertSmartConnectParameters(int bandNum){
 (void)bandNum;if(nvram_get_int("dwb_scb")!=1)return;
 int band=nvram_get_int("dwb_band");char backup[128],key[128];
 for(unsigned i=0;i<4;i++){
  const char *param=sc_basic_param[i];if(!param)continue;
  snprintf(backup,sizeof(backup),"scb_%s",param);
  if(!strcmp(param,"bsd_ifnames"))nvram_set(param,get(backup));
  else if(!strcmp(param,"smart_connect_x")&&nvram_get_int("amas_eap_bhmode")<=0&&nvram_get_int("scb_smart_connect_x")==2&&nvram_get_int("smart_connect_x")==0)nvram_set(param,get(backup));
  nvram_unset(backup);
 }
 unsigned count=interfaces();
 for(unsigned i=0;i<count;i++){
  if((int)i==band)continue;
  for(unsigned j=0;sc_detailed_param[j];j++){
   const char *param=sc_detailed_param[j];snprintf(key,sizeof(key),"wl%u_%s",i,param);snprintf(backup,sizeof(backup),"scb_wl%u_%s",i,param);
   if(!strcmp(param,"bsd_if_select_policy"))nvram_set(key,get(backup));
   nvram_unset(backup);
  }
 }
 nvram_unset("dwb_scb");
}
