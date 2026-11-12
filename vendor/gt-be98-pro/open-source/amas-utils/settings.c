#include <stdio.h>
#include <string.h>
extern int nvram_get_int(const char *),nvram_set_int(const char *,int),nvram_unset(const char *),num_of_wl_if(void);
void add_led_ctrl_capability(int bits){nvram_set_int("led_ctrl_cap",nvram_get_int("led_ctrl_cap")|bits);}
void del_led_ctrl_capability(int bits){int current=nvram_get_int("led_ctrl_cap");if(current>0&&(bits&~current)==0)nvram_set_int("led_ctrl_cap",current&~bits);}
int dwb_band_exist(void){return num_of_wl_if()!=2;}
int is_connection_priority_is_wifi_first(void){int mode=nvram_get_int("amas_ethernet");return mode>=100&&mode<=400;}
void unset_selected_channel_info(void){
 int count=num_of_wl_if();const char *suffixes[]={"sel_channel","sel_bw","sel_nctrlsb"};
 for(int unit=0;unit<count;unit++){
  char prefix[9],key[32],full[32];snprintf(full,sizeof(full),"wl%d_",unit);
  size_t length=strlen(full);if(length>8)length=8;memcpy(prefix,full,length);prefix[length]=0;
  for(unsigned i=0;i<3;i++){snprintf(key,sizeof(key),"%s%s",prefix,suffixes[i]);nvram_unset(key);}
 }
}
