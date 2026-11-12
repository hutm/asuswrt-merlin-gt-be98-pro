/* SPDX-License-Identifier: GPL-2.0-only */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "model.h"
#include <net/if.h>
extern char *wl_nvname(const char *,int,int),*nvram_get(const char *);
extern int f_read_string(const char *,char *,int);
static char *query(int band,int model){
 static char values[3][32];char *value=values[band];const char *iface=NULL;int unit=-1;
 memset(value,0,32);
 switch(band){
 case 0:
  switch(model){
   case MODEL_GTAC5300:case MODEL_RTAX88U:case MODEL_GTAX11000:case MODEL_GTAXE11000:case MODEL_GTAX11000_PRO:case MODEL_RTAX86U_PRO:case MODEL_GTAX6000:case MODEL_RTAX88U_PRO: iface="eth6";break;
   case MODEL_RTAC86U:case MODEL_RTAX92U: iface="eth5";break;
   case MODEL_RPBE58:case MODEL_RTBE96U:case MODEL_BT12:case MODEL_GTBE96:case MODEL_RTBE88U:case MODEL_GTBE19000:case MODEL_BT10:case MODEL_RTBE58_GO:case MODEL_GTBE19000AI:case MODEL_GTBE96_AI: iface="wl0";break;
   case MODEL_GTAXE16000: iface="eth10";break;
   case MODEL_GTBE98:case MODEL_GTBE98_PRO:case MODEL_BQ16:case MODEL_BQ16_PRO: iface="wl3";break;
   case MODEL_RTBE86U:case MODEL_RTBE58U:case MODEL_RTBE92U:case MODEL_RTBE82U:case MODEL_RTBE58U_PRO:case MODEL_RTBE82M:case MODEL_RTBE58U_V2: unit=0;break;
   case MODEL_RTBE95U:case MODEL_GSBE18000:case MODEL_GT7: unit=2;break;
   default: iface="eth1";break;
  }break;
 case 1:
  switch(model){
   case MODEL_GTAC5300:case MODEL_RTAX88U:case MODEL_GTAX11000:case MODEL_GTAXE11000:case MODEL_GTAX11000_PRO:case MODEL_RTAX86U_PRO:case MODEL_GTAX6000:case MODEL_GTAXE16000:case MODEL_RTAXE7800:case MODEL_RTAX88U_PRO: iface="eth7";break;
   case MODEL_RTAC86U: iface="eth6";break;
   case MODEL_RTAX92U: iface="eth5";break;
   case MODEL_RPBE58:case MODEL_RTBE96U:case MODEL_BT12:case MODEL_GTBE96:case MODEL_RTBE88U:case MODEL_GTBE19000:case MODEL_BT10:case MODEL_RTBE58_GO:case MODEL_GTBE19000AI:case MODEL_GTBE96_AI: iface="wl1";break;
   case MODEL_GTBE98:case MODEL_GTBE98_PRO:case MODEL_BQ16:case MODEL_BQ16_PRO: iface="wl0";break;
   case MODEL_RTBE86U:case MODEL_RTBE58U:case MODEL_RTBE92U:case MODEL_RTBE82U:case MODEL_RTBE58U_PRO:case MODEL_RTBE82M:case MODEL_GSBE18000:case MODEL_RTBE58U_V2:case MODEL_GT7: unit=1;break;
   case MODEL_RTBE95U: unit=0;break;
   default: iface="eth2";break;
  }break;
 case 2:
  switch(model){
   case MODEL_RTAC5300: iface="eth3";break;
   case MODEL_GTAC5300:case MODEL_GTAX11000:case MODEL_GTAXE11000:case MODEL_GTAX11000_PRO:case MODEL_GTAXE16000: iface="eth8";break;
   case MODEL_RTAX92U:case MODEL_RTAX9000: iface="eth7";break;
   case MODEL_GT10: iface="eth5";break;
   case MODEL_GTBE98:case MODEL_GTBE98_PRO:case MODEL_BQ16:case MODEL_BQ16_PRO: iface="wl1";break;
   case MODEL_RTBE96U:case MODEL_BT12:case MODEL_GTBE96:case MODEL_GTBE19000:case MODEL_BT10:case MODEL_GTBE19000AI:case MODEL_GTBE96_AI: iface="wl2";break;
   case MODEL_RTBE92U: unit=2;break;
   case MODEL_RTBE95U: unit=1;break;
   case MODEL_GSBE18000:case MODEL_GT7: unit=0;break;
   default: strcpy(value,"0");return value;
  }break;
 }
 if(unit>=0){iface=nvram_get(wl_nvname("ifname",unit,0));if(!iface)iface="";}
 /* Bound and validate the interface before using it as a shell argument. */
 size_t length=strnlen(iface,IFNAMSIZ);
 if(!length||length>=IFNAMSIZ)return value;
 for(size_t i=0;i<length;i++){
  unsigned char c=iface[i];
  if(!((c>='a'&&c<='z')||(c>='A'&&c<='Z')||(c>='0'&&c<='9')||c=='_'||c=='-'||c=='.'||c==':'))return value;
 }
 char command[128];snprintf(command,sizeof(command),"wl -i %s clm_data_ver > /tmp/hw_auth_clm",iface);
 int status=system(command);(void)status;
 if(f_read_string("/tmp/hw_auth_clm",value,32)<=0)memset(value,0,32);
 return value;
}
char *hw_component_clm_2g(int model){return query(0,model);}
char *hw_component_clm_5g1(int model){return query(1,model);}
char *hw_component_clm_5g2(int model){return query(2,model);}
