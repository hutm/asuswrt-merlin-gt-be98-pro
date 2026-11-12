/* SPDX-License-Identifier: GPL-2.0-only */
#include <stdio.h>
#include <string.h>
extern char *get_productid(void),*nvram_get(const char *);
extern int get_model(void);
extern char *hw_component_clm_2g(int),*hw_component_clm_5g1(int),*hw_component_clm_5g2(int);
char *DoHardwareComponent(char *index){
 static char value[32];char *product=get_productid();const char *source="";
 memset(value,0,sizeof(value));
 if(!strcmp(index,"productid")||!strcmp(index,"odmpid")){
  source=!strncmp(product,"RT-AC68",7)&&strncmp(product,"RT-AC68U_V4",11)?"RT-AC68":product;
 }else if(!strcmp(index,"btn_rst_gpio")||!strcmp(index,"btn_wps_gpio")||!strcmp(index,"btn_led_gpio")||!strcmp(index,"btn_wltog_gpio")){
  source=nvram_get(index);if(!source)source="";
 }else if(!strcmp(index,"clm_data_ver_2g"))source=hw_component_clm_2g(get_model());
 else if(!strcmp(index,"clm_data_ver_5g1"))source=hw_component_clm_5g1(get_model());
 else if(!strcmp(index,"clm_data_ver_5g2"))source=hw_component_clm_5g2(get_model());
 snprintf(value,sizeof(value),"%s",source);return value;
}
