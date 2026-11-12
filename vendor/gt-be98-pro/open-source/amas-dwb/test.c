/* SPDX-License-Identifier: GPL-2.0-only */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
struct connect_param_mapping_s {char *param;};
extern struct connect_param_mapping_s connect_param_mapping_list[];
extern const char *sc_basic_param[],*sc_detailed_param[];
extern void cm_revertSmartConnectParameters(int);
extern int cm_revert_wifi_parameters(void);
extern void dwb_init_settings(void);
struct value {char key[128],value[512];int present;};
static struct value values[512];static unsigned count;static int write_error;
static struct value *find(const char *key){for(unsigned i=0;i<count;i++)if(!strcmp(values[i].key,key))return &values[i];return NULL;}
static void put(const char *key,const char *value){struct value *v=find(key);if(!v){assert(count<512);v=&values[count++];assert(strlen(key)<sizeof(v->key));strcpy(v->key,key);}assert(strlen(value)<sizeof(v->value));memmove(v->value,value,strlen(value)+1);v->present=1;}
char *nvram_get(const char *key){struct value *v=find(key);printf("get %s\n",key);return v&&v->present?v->value:NULL;}
int nvram_get_int(const char *key){struct value *v=find(key);printf("get-int %s\n",key);return v&&v->present?atoi(v->value):0;}
int nvram_set(const char *key,const char *value){printf("set %s=[%s]\n",key,value);if(!write_error)put(key,value);return write_error;}
int nvram_set_int(const char *key,int value){char s[32];snprintf(s,sizeof(s),"%d",value);printf("set-int %s=[%s]\n",key,s);if(!write_error)put(key,s);return write_error;}
int nvram_unset(const char *key){printf("unset %s\n",key);struct value *v=find(key);if(v&&!write_error)v->present=0;return write_error;}
int nvram_commit(void){puts("commit");return write_error;}
size_t strlcpy(char *dst,const char *src,size_t cap){size_t n=strlen(src);if(cap){size_t k=n<cap-1?n:cap-1;memcpy(dst,src,k);dst[k]=0;}return n;}
static void seed(void){
 count=0;put("dwb_scb","1");put("dwb_band","2");put("wl_ifnames","eth1 eth2 eth3 eth4");put("dwb_band_ori_ssid","saved-ssid");put("dwb_band_ori_closed","1");put("amas_dwb_rule","0");put("mlo_re_mssid_subunit","3");put("mlo_dwb_mssid_subunit","4");
 for(unsigned i=0;sc_basic_param[i];i++){char key[128];snprintf(key,sizeof(key),"scb_%s",sc_basic_param[i]);put(key,!strcmp(sc_basic_param[i],"bsd_ifnames")?"eth1 eth2 eth3 eth4":"saved-basic");}
 for(int band=0;band<4;band++)for(unsigned i=0;sc_detailed_param[i];i++){char key[128],v[128];snprintf(key,sizeof(key),"scb_wl%d_%s",band,sc_detailed_param[i]);snprintf(v,sizeof(v),"saved-%d-%u",band,i);put(key,v);}
}
int main(void){
 for(unsigned i=0;connect_param_mapping_list[i].param;i++){assert(i<32);printf("mapping %s\n",connect_param_mapping_list[i].param);}
 for(unsigned i=0;sc_basic_param[i];i++){assert(i<16);printf("basic %s\n",sc_basic_param[i]);}
 for(unsigned i=0;sc_detailed_param[i];i++){assert(i<16);printf("detailed %s\n",sc_detailed_param[i]);}
 seed();puts("CASE smart");cm_revertSmartConnectParameters(4);
 seed();puts("CASE restore");printf("result %d\n",cm_revert_wifi_parameters());
 char *bands[]={NULL,"","-1","0","1","2","3","4"};char *lists[]={"","eth1","eth1 eth2","eth1 eth2 eth3 eth4"," eth1  eth2 ","eth1\teth2"};
 for(unsigned b=0;b<8;b++)for(unsigned l=0;l<6;l++)for(int rule=0;rule<2;rule++)for(int re=0;re<2;re++){
  seed();struct value *v=find("dwb_band");v->present=0;if(bands[b])put("dwb_band",bands[b]);put("wl_ifnames",lists[l]);put("amas_dwb_rule",rule?"1":"0");put("re_mode",re?"1":"0");
  printf("CASE init %u %u %d %d\n",b,l,rule,re);dwb_init_settings();
 }
 for(unsigned b=0;b<8;b++)for(unsigned l=0;l<6;l++)for(int enabled=-1;enabled<3;enabled++)for(int eap=-1;eap<2;eap++)for(int old=0;old<4;old++)for(int cur=0;cur<3;cur++){
  seed();find("dwb_band")->present=0;if(bands[b])put("dwb_band",bands[b]);put("wl_ifnames",lists[l]);
  char v[32];snprintf(v,sizeof(v),"%d",enabled);put("dwb_scb",v);snprintf(v,sizeof(v),"%d",eap);put("amas_eap_bhmode",v);snprintf(v,sizeof(v),"%d",old);put("scb_smart_connect_x",v);snprintf(v,sizeof(v),"%d",cur);put("smart_connect_x",v);
  printf("CASE smart-matrix %u %u %d %d %d %d\n",b,l,enabled,eap,old,cur);cm_revertSmartConnectParameters(-1);
 }
 for(unsigned b=0;b<8;b++)for(int saved=0;saved<3;saved++){
  seed();find("dwb_band")->present=0;if(bands[b])put("dwb_band",bands[b]);if(saved==0)find("dwb_band_ori_ssid")->present=0;else if(saved==1)put("dwb_band_ori_ssid","");
  printf("CASE restore-matrix %u %d\n",b,saved);printf("result %d\n",cm_revert_wifi_parameters());
 }
 for(int failure=0;failure<2;failure++)for(unsigned length=0;length<80;length++){
  seed();write_error=failure?-1:0;char list[128];memset(list,'x',length);list[length]=0;for(unsigned i=2;i<length;i+=4)list[i]=' ';put("wl_ifnames",list);put("dwb_band","");
  printf("CASE truncation %d %u\n",failure,length);dwb_init_settings();cm_revertSmartConnectParameters(0);printf("result %d\n",cm_revert_wifi_parameters());
 }
 return 0;
}
