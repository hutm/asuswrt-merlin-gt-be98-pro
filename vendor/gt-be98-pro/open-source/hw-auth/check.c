/* SPDX-License-Identifier: GPL-2.0-only */
#include <stdlib.h>
#include <string.h>
struct hw_entry {char *field[9];};
extern char *get_productid(void),*DoHardwareComponent(char *);
static int gpio(const char *a,const char *b){return (((unsigned)atoi(a)^(unsigned)atoi(b))&4095u)==0;}
static int compare(const struct hw_entry *entry){
 char **v=(char **)entry->field;
 int clm0=!strncmp(v[5],DoHardwareComponent("clm_data_ver_2g"),strlen(v[5]));
 int clm1=*v[6]?!strncmp(v[6],DoHardwareComponent("clm_data_ver_5g1"),strlen(v[6])):0;
 int clm2=*v[7]?!strncmp(v[7],DoHardwareComponent("clm_data_ver_5g2"),strlen(v[7])):0;
 int product=!strcmp(v[0],DoHardwareComponent("odmpid"));
 int reset=gpio(v[1],DoHardwareComponent("btn_rst_gpio"));
 int wps=gpio(v[2],DoHardwareComponent("btn_wps_gpio"));
 int led=gpio(v[3],DoHardwareComponent("btn_led_gpio"));
 int toggle=gpio(v[4],DoHardwareComponent("btn_wltog_gpio"));
 int hw=!strcmp(v[8],DoHardwareComponent("hwcode"));
 return product&&reset&&wps&&led&&toggle&&(clm0||clm1||clm2)&&hw;
}
static int internal_check(struct hw_entry *table){
 const char *product=get_productid();
 if(!strncmp(product,"RT-AC68",7)&&strncmp(product,"RT-AC68U_V4",11))product="RT-AC68";
 for(;table->field[0];table++)if(!strcmp(table->field[0],product))return compare(table);
 return 0;
}
#include <stdio.h>
#include <time.h>
struct sw_entry {char *name,*id,*key;unsigned flags;};
char *gen_rand_value(char *exclude){
 static char value[64];memset(value,0,sizeof(value));
 do {srand(time(NULL));snprintf(value,sizeof(value),"%d",rand());}while(!strcmp(value,exclude));
 return value;
}
static char *internal_hardware(char *key,struct sw_entry *software,struct hw_entry *hardware){
 static char result[64];char id[32]={0};const char *product=get_productid();int success=0;
 memset(result,0,sizeof(result));
 for(;software->key;software++)if(!strcmp(software->key,key)){snprintf(id,sizeof(id),"%s",software->id);break;}
 if(*id){
  if(!strncmp(product,"RT-AC68",7)&&strncmp(product,"RT-AC68U_V4",11))product="RT-AC68";
  for(;hardware->field[0];hardware++)if(!strcmp(hardware->field[0],product)){success=compare(hardware);break;}
 }
 snprintf(result,sizeof(result),"%s",success==1?id:gen_rand_value(id));return result;
}

extern struct hw_entry s_hw_auth_tuple[];
extern struct sw_entry s_sw_auth_tuple[];
int HwCheckResult(void){return internal_check(s_hw_auth_tuple);}
char *DoHardwareCheck(char *key){return internal_hardware(key,s_sw_auth_tuple,s_hw_auth_tuple);}
