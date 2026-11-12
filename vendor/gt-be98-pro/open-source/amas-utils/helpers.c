#include <stdio.h>
extern int num_of_wl_if(void),nvram_get_int(const char *);
extern char *wl_nvname(const char *,int,int),*nvram_get(const char *);
float cal_plc_cost(float parent,int rate){
 float weight=rate>=900?1.0f:rate>=400?2.0f:rate>=200?4.0f:8.0f;
 return (parent+weight)+0.0f;
}
int get_bandnum_by_nband(int band){
 int count=num_of_wl_if(),matches=0;
 for(int unit=0;unit<count;unit++){
  if(unit>3||!nvram_get(wl_nvname("nband",unit,0))){unit++;continue;}
  char key[64];snprintf(key,sizeof(key),"wl%d_nband",unit);
  if(nvram_get_int(key)==band)matches++;
 }
 return matches;
}
#include <string.h>
static int interface_count(const char *key){
 const char *value=nvram_get(key);char text[32];snprintf(text,sizeof(text),"%s",value?value:"");
 char *p=text;int count=0;
 while(1){p+=strspn(p," ");if(!*p)return count;count++;char *next=strchr(p,' ');if(!next)return count;p=next;}
}
int get_wl_count(void){return interface_count("sta_ifnames");}
int get_eth_count(void){return interface_count("eth_ifnames");}
