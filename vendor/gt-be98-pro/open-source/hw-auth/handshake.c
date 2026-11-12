/* SPDX-License-Identifier: GPL-2.0-only */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
struct sw_entry {char *name,*id,*key;unsigned flags;};
struct mode_entry {char *product;int mode;};
extern struct sw_entry s_sw_auth_tuple[];
extern struct mode_entry s_amas_whitelist_tuple[];
extern char *s_tunnel_whitelist_tuple[];
extern char *get_lan_hwaddr(void),*get_wan_hwaddr(void),*get_2g_hwaddr(void),*get_productid(void);
extern int ether_atoe(const char *,unsigned char *),f_exists(const char *),nvram_get_int(const char *);
extern char *gen_rand_value(char *),*DoHardwareCheck(char *),*get_auth_code(char *,char *,int);
#include "prefixes.h"
static int listed(const unsigned char *mac){
 for(unsigned i=0;prefixes[i];i++){
  uint32_t value=prefixes[i]<<3;
  if(mac[0]==(unsigned char)(value>>24)&&mac[1]==(unsigned char)(value>>16)&&mac[2]==(unsigned char)(value>>8))return 1;
 }
 return 0;
}
static int family(const char *name){return !strncmp(name,"RT-AC68",7)&&strncmp(name,"RT-AC68U_V4",11);}
static const struct sw_entry *find_app(const char *id){
 for(const struct sw_entry *p=s_sw_auth_tuple;p->id;p++)if(!strcmp(p->id,id))return p;
 return NULL;
}
char *hw_auth_check(char *id,char *code,time_t timestamp,char *out,int cap){
 unsigned char lan[6]={0},wan[6]={0},wifi[6]={0};char key[64]={0},name[32]={0},message[128];int is68=0,allowed=0;
 if(!id||!code||!out||cap<65||cap>513){errno=EINVAL;return NULL;}
 ether_atoe(get_lan_hwaddr(),lan);ether_atoe(get_wan_hwaddr(),wan);ether_atoe(get_2g_hwaddr(),wifi);
 int blocked=listed(lan)|listed(wan)|listed(wifi);f_exists("/tmp/HW_AUTH_DEBUG");if(blocked)goto failure;
 blocked=f_exists("/jffs/.sys/RT-AC68U/unlock")!=0;if(!blocked)blocked=nvram_get_int("fw_check")!=0;
 f_exists("/tmp/HW_AUTH_DEBUG");if(blocked)goto failure;
 const struct sw_entry *entry=find_app(id);if(entry)snprintf(key,sizeof(key),"%s",entry->key);if(!*key)goto failure;
 entry=find_app(id);if(entry)snprintf(name,sizeof(name),"%s",entry->name);
 const char *product=get_productid();is68=family(product);
 if(!strcmp(name,"AMASH")){
  for(struct mode_entry *p=s_amas_whitelist_tuple;p->product;p++)if(is68||!strcmp(p->product,product)){allowed=1;break;}
  f_exists("/tmp/HW_AUTH_DEBUG");if(!allowed)goto failure;
 }
 memset(name,0,sizeof(name));entry=find_app(id);if(entry)snprintf(name,sizeof(name),"%s",entry->name);
 product=get_productid();if(family(product))is68=1;
 if(!strcmp(name,"aaews")||!strcmp(name,"mastiff")){
  allowed=0;for(char **p=s_tunnel_whitelist_tuple;*p;p++)if(is68||!strcmp(*p,product)){allowed=1;break;}
  f_exists("/tmp/HW_AUTH_DEBUG");if(!allowed)goto failure;
  while(!nvram_get_int("wlready"))sleep(1);
 }
 snprintf(message,sizeof(message),"%ld|%s",(long)timestamp,key);
 char *generated=get_auth_code(message,out,cap);if(!generated)return NULL;
 if(strcmp(code,generated))goto failure;
 snprintf(message,sizeof(message),"%ld|%s|%s",(long)timestamp,key,DoHardwareCheck(key));
 return get_auth_code(message,out,cap);
failure:
 snprintf(message,sizeof(message),"%ld|%s|%s",(long)timestamp,key,gen_rand_value(id));
 return get_auth_code(message,out,cap);
}
