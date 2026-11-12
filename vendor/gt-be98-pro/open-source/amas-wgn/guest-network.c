#define _GNU_SOURCE
#include "subnet.h"
#include "vlan.h"

/* subnet.c */
#include <stdlib.h>
#include <string.h>
#include <errno.h>
static int32_t number(const char *text){long long n=strtoll(text,NULL,10);if(n>INT32_MAX){errno=ERANGE;return INT32_MAX;}if(n<INT32_MIN){errno=ERANGE;return INT32_MIN;}return n;}
static void field(char *out,size_t size,const char *in){size_t n=strlen(in);if(n&&n<size)memcpy(out,in,n+1);}
wgn_subnet_rule *wgn_subnet_list_get_from_content(char *content,wgn_subnet_rule *list,size_t capacity,size_t *count,int cfg){
 if(count)*count=0;
 if(!content||!list||!capacity)return NULL;
 char *copy=strdup(content);if(!copy)return NULL;
 char *next=copy,*row;size_t used=0;
 while((row=strsep(&next,"<"))){
  char *tokens[12],*cursor=row;unsigned n;
  for(n=0;n<12;n++)if(!(tokens[n]=strsep(&cursor,">")))break;
  if(n!=12)continue;
  if(cfg==1&&number(tokens[11])!=1)continue;
  if(cfg==2&&number(tokens[11])==1)continue;
  if(used==capacity)break;
  wgn_subnet_rule *r=&list[used++];
  field(r->ipaddr,sizeof(r->ipaddr),tokens[0]);field(r->ipmask,sizeof(r->ipmask),tokens[1]);
  if(*tokens[2])r->dhcp_enable=number(tokens[2])!=0;
  field(r->dhcp_start,sizeof(r->dhcp_start),tokens[3]);field(r->dhcp_end,sizeof(r->dhcp_end),tokens[4]);
  if(*tokens[5])r->dhcp_lease=number(tokens[5]);
  field(r->domain_name,sizeof(r->domain_name),tokens[6]);field(r->dns,sizeof(r->dns),tokens[7]);field(r->wins,sizeof(r->wins),tokens[8]);
  if(*tokens[9])r->ema=number(tokens[9]);
  field(r->macipbinding,sizeof(r->macipbinding),tokens[10]);r->type=number(tokens[11]);
 }
 if(count)*count=used;
 free(copy);return list;
}
#include <stdio.h>
static void append(char *out,size_t capacity,const char *in){size_t used=strnlen(out,capacity);if(used==capacity)return;size_t n=strlen(in);if(n>capacity-used-1)n=capacity-used-1;memcpy(out+used,in,n);out[used+n]=0;}
static void string_field(char *out,size_t capacity,const char *value){char formatted[32];snprintf(formatted,sizeof(formatted),"%s>",value);append(out,capacity,formatted);}
static void integer_field(char *out,size_t capacity,unsigned value){char formatted[32];int32_t signed_value;memcpy(&signed_value,&value,4);snprintf(formatted,sizeof(formatted),"%d>",signed_value);append(out,capacity,formatted);}
char *wgn_subnet_list_set_to_buffer(wgn_subnet_rule *list,size_t count,char *buffer,size_t capacity){
 if(!list||!count||!buffer||!capacity||capacity>8192)return NULL;
 for(size_t i=0;i<count;i++){
  wgn_subnet_rule *r=&list[i];append(buffer,capacity,"<");string_field(buffer,capacity,r->ipaddr);string_field(buffer,capacity,r->ipmask);
  integer_field(buffer,capacity,r->dhcp_enable);string_field(buffer,capacity,r->dhcp_start);string_field(buffer,capacity,r->dhcp_end);
  integer_field(buffer,capacity,r->dhcp_lease);string_field(buffer,capacity,r->domain_name);string_field(buffer,capacity,r->dns);string_field(buffer,capacity,r->wins);
  integer_field(buffer,capacity,r->ema);string_field(buffer,capacity,r->macipbinding);integer_field(buffer,capacity,r->type);
 }
 return buffer;
}

/* basic.c */
#include <stdint.h>
#include <string.h>
int compare(const void *a,const void *b){uint32_t x,y,result;int32_t signed_result;memcpy(&x,a,4);memcpy(&y,b,4);result=x-y;memcpy(&signed_result,&result,4);return signed_result;}
int ipmask_to_int(uint32_t mask){unsigned prefix=0;while(prefix<32&&(mask&(UINT32_C(0x80000000)>>prefix)))prefix++;return prefix>=31?0:(int)prefix;}

/* subnet-settings.c */
#include <arpa/inet.h>
extern char *nvram_get(const char *);
extern int nvram_set(const char *,const char *),nvram_commit(void);
static wgn_subnet_rule *read_subnet_settings(wgn_subnet_rule *list,size_t capacity,size_t *count,int cfg){
 if(count)*count=0;
 if(!list||!capacity)return NULL;
 char *value=nvram_get("subnet_rulelist");char *copy=strdup(value?value:"");if(!copy)return NULL;
 wgn_subnet_rule *result=wgn_subnet_list_get_from_content(copy,list,capacity,count,cfg);free(copy);return result;
}
wgn_subnet_rule *wgn_subnet_list_get_from_nvram(wgn_subnet_rule *list,size_t capacity,size_t *count){return read_subnet_settings(list,capacity,count,1);}
wgn_subnet_rule *wgn_subnet_list_get_all_from_nvram(wgn_subnet_rule *list,size_t capacity,size_t *count){return read_subnet_settings(list,capacity,count,0);}
wgn_subnet_rule *wgn_subnet_list_get_other_from_nvram(wgn_subnet_rule *list,size_t capacity,size_t *count){return read_subnet_settings(list,capacity,count,2);}
wgn_subnet_rule *wgn_subnet_list_find(wgn_subnet_rule *list,size_t count,char *name){
 if(!list||!count||!name)return NULL;
 for(size_t i=0;i<count;i++){char key[33]={0};snprintf(key,sizeof(key),"%s/%d",list[i].ipaddr,ipmask_to_int(inet_network(list[i].ipmask)));if(!strncmp(key,name,strlen(name)))return &list[i];}
 return NULL;
}
void wgn_subnet_list_set_to_nvram(wgn_subnet_rule *list,size_t count){
 if(!list||!count)return;
 if(count>8){errno=ENOSPC;return;}
 wgn_subnet_rule *merged=malloc(8*sizeof(*merged));if(!merged)return;
 char *buffer=malloc(8193);if(!buffer){free(merged);return;}
 memset(merged,0,8*sizeof(*merged));size_t retained=0;
 if(!wgn_subnet_list_get_other_from_nvram(merged,8,&retained)){free(merged);free(buffer);return;}
 if(count>8-retained){free(merged);free(buffer);errno=ENOSPC;return;}
 for(size_t i=0;i<count;i++)if(list[i].type==1)memcpy(&merged[retained+i],&list[i],sizeof(*list));
 memset(buffer,0,8193);
 if(wgn_subnet_list_set_to_buffer(merged,retained+count,buffer,8192)){nvram_set("subnet_rulelist",buffer);nvram_commit();}
 free(merged);free(buffer);
}

/* vlan.c */
char *wgn_vlan_list_set_to_buffer(wgn_vlan_rule *list,size_t count,char *buffer,size_t capacity){
 if(!list||!count||!buffer||!capacity||capacity>8192)return NULL;
 for(size_t i=0;i<count;i++){
  wgn_vlan_rule *r=&list[i];append(buffer,capacity,"<");
  integer_field(buffer,capacity,r->enable);integer_field(buffer,capacity,r->vid);integer_field(buffer,capacity,(unsigned char)r->prio);
  string_field(buffer,capacity,r->wanportset);string_field(buffer,capacity,r->lanportset);
  string_field(buffer,capacity,r->wl2gset);string_field(buffer,capacity,r->wl5gset);string_field(buffer,capacity,r->subnet_name);
  integer_field(buffer,capacity,r->internet);integer_field(buffer,capacity,r->public_vlan);integer_field(buffer,capacity,r->type);
 }
 return buffer;
}
wgn_vlan_rule *wgn_vlan_list_find(wgn_vlan_rule *list,size_t count,char *name){
 if(!list||!count||!name)return NULL;
 for(size_t i=0;i<count;i++)if(!strncmp(list[i].subnet_name,name,strlen(name)))return &list[i];
 return NULL;
}
extern int wgn_get_wl_band_unit(int *,int *,char *,char *);
extern int wgn_guest_ifcount(int);
wgn_vlan_rule *wgn_vlan_list_get_from_content(char *content,wgn_vlan_rule *list,size_t capacity,size_t *count,int cfg){
 if(count)*count=0;
 if(!content||!list||!capacity)return NULL;
 char *copy=strdup(content);if(!copy)return NULL;
 char *next=copy,*row;size_t used=0;int band_counts[64]={0};
 while((row=strsep(&next,"<"))){
  char *tokens[11],*cursor=row;unsigned n;
  for(n=0;n<11;n++)if(!(tokens[n]=strsep(&cursor,">")))break;
  if(n!=11)continue;
  if(cfg==1){
   if(number(tokens[10])!=1)continue;
   int band=wgn_get_wl_band_unit(NULL,NULL,tokens[5],tokens[6]);
   if(band<0||band>=64)continue;
   if(band_counts[band]>=wgn_guest_ifcount(band))continue;
   band_counts[band]++;
  }else if(cfg==2&&number(tokens[10])==1)continue;
  if(used==capacity)break;
  wgn_vlan_rule *r=&list[used++];
  if(*tokens[0])r->enable=number(tokens[0]);
  if(*tokens[1])r->vid=number(tokens[1]);
  if(*tokens[2])r->prio=(char)number(tokens[2]);
  field(r->wanportset,sizeof(r->wanportset),tokens[3]);field(r->lanportset,sizeof(r->lanportset),tokens[4]);
  field(r->wl2gset,sizeof(r->wl2gset),tokens[5]);field(r->wl5gset,sizeof(r->wl5gset),tokens[6]);field(r->subnet_name,sizeof(r->subnet_name),tokens[7]);
  if(*tokens[8])r->internet=number(tokens[8]);
  if(*tokens[9])r->public_vlan=number(tokens[9]);
  r->type=number(tokens[10]);
 }
 if(count)*count=used;
 free(copy);return list;
}

/* bands.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern char *nvram_get(const char *);
extern int nvram_get_int(const char *);
extern char *wl_nvname(const char *,int,int);
int wgn_get_unit_by_band(int band){
 char names[256],key[64];const char *stored=nvram_get("wl_ifnames");
 snprintf(names,sizeof(names),"%s",stored?stored:"");char *p=names;int unit=0;
 while(1){
  p+=strspn(p," ");if(!*p)return -1;
  char *next=strchr(p,' ');
  if(unit<=3 && nvram_get(wl_nvname("nband",unit,0))){
   snprintf(key,sizeof(key),"wl%d_nband_type",unit);
   int match=0;
   switch(band){
   case 0:match=nvram_get_int(key)==0;break;
   case 1:match=nvram_get_int(key)==1||nvram_get_int(key)==2;break;
   case 2:match=nvram_get_int(key)==3;break;
   case 3:match=nvram_get_int(key)==4||nvram_get_int(key)==5;break;
   case 4:match=nvram_get_int(key)==6;break;
   }
   if(match)return unit;
  }
  unit++;p=next?next:p+strlen(p);
 }
}
int wgn_get_band_by_unit(int unit){
 char key[64];snprintf(key,63,"wl%d_nband_type",unit);
 if(!nvram_get(key))return -1;
 int type=nvram_get_int(key);static const signed char bands[]={0,1,1,2,3,3,4};
 return (unsigned)type<7?bands[type]:-1;
}
static int decode(int *unit,int *subunit,char *two,char *five,int max_band){
 if(unit)*unit=-1;
 if(subunit)*subunit=-1;
 int band=-1;uint32_t bits=0;
 if(two&&strlen(two)==4){bits=(uint32_t)strtoul(two,NULL,10);if(bits&&bits<=INT32_MAX)band=0;}
 if(band<0&&five){
  size_t len=strlen(five);
  if(len>=4&&len<=(size_t)max_band*4&&len%4==0){
   bits=(uint32_t)strtoul(five+len-4,NULL,10);
   if(bits&&bits<=INT32_MAX)band=(int)(len/4);
  }
 }
 if(band<0)return -1;
 int mapped=wgn_get_unit_by_band(band);if(mapped==-1)return -1;
 unsigned sub=0;while(sub<16&&!(bits&(1u<<sub)))sub++;
 if(unit)*unit=mapped;
 if(subunit)*subunit=(int)sub;
 return mapped;
}
int wgn_get_wl_band_unit(int *unit,int *subunit,char *two,char *five){return decode(unit,subunit,two,five,3);}
void wgn_get_wl_unit(int *unit,int *subunit,wgn_vlan_rule *rule){decode(unit,subunit,rule?rule->wl2gset:NULL,rule?rule->wl5gset:NULL,4);}
int wgn_guest_ifcount(int band){(void)band;return 1;}

/* vlan-units.c */
void wgn_set_wl_unit(int unit,int subunit,wgn_vlan_rule *rule){
 if(!rule||unit<0||subunit<1||subunit>15)return;
 int band=wgn_get_band_by_unit(unit);char *out;
 if(band==0)out=rule->wl2gset;
 else if(band>=1&&band<=3)out=rule->wl5gset+4*(band-1);
 else return;
 snprintf(out,32,"%04X",1u<<subunit);
}
wgn_vlan_rule *wgn_vlan_list_find_unit(wgn_vlan_rule *list,size_t count,int unit,int subunit){
 if(!list||!count||unit<0||subunit<=0)return NULL;
 for(size_t i=0;i<count;i++){int u,s;wgn_get_wl_unit(&u,&s,&list[i]);if(u==unit&&s==subunit)return &list[i];}
 return NULL;
}
int wgn_vlan_list_check_subunit_conflict(wgn_vlan_rule *list,size_t count,int unit,int subunit){
 if(!list||!count||unit<0||subunit<=0)return 0;
 int subs[8];size_t n=0;
 for(size_t i=0;i<count&&n<8;i++){int u,s;wgn_get_wl_unit(&u,&s,&list[i]);if(u==unit&&s>0)subs[n++]=s;}
 qsort(subs,n,sizeof(*subs),compare);int conflict=0;
 for(size_t i=0;i<n;i++)if(subs[i]==subunit){subs[i]++;conflict=1;}
 if(!conflict)return 0;
 for(size_t i=0;i<n;i++)for(size_t j=i+1;j<n;j++)if(subs[j]==subs[i])subs[j]=subs[i]+1;
 size_t used=0;
 for(size_t i=0;i<count&&used<n;i++){int u,s;wgn_get_wl_unit(&u,&s,&list[i]);if(u==unit&&s>0)wgn_set_wl_unit(unit,subs[used++],&list[i]);}
 return 1;
}
void wgn_vlan_list_wl_subunit_shift(wgn_vlan_rule *list,size_t count){
 if(!list||!count)return;
 const char *value=nvram_get("wl_ifnames"),*again=nvram_get("wl_ifnames");
 const char *p=value?value:"";p+=strspn(again?again:""," ");
 int unit=0;
 while(*p){
  const char *next=strchr(p,' ');
  if(unit<=3&&nvram_get(wl_nvname("nband",unit,0)))wgn_vlan_list_check_subunit_conflict(list,count,unit,1);
  unit++;p=next?next+strspn(next," "):p+strlen(p);
 }
}

/* vlan-settings.c */
extern char *nvram_get(const char *);
extern int nvram_set(const char *,const char *),nvram_commit(void);
static wgn_vlan_rule *read_vlan_settings(wgn_vlan_rule *list,size_t capacity,size_t *count,int cfg){
 if(count)*count=0;
 if(!list||!capacity)return NULL;
 char *value=nvram_get("vlan_rulelist");char *copy=strdup(value?value:"");if(!copy)return NULL;
 wgn_vlan_rule *result=wgn_vlan_list_get_from_content(copy,list,capacity,count,cfg);free(copy);return result;
}
wgn_vlan_rule *wgn_vlan_list_get_from_nvram(wgn_vlan_rule *list,size_t capacity,size_t *count){return read_vlan_settings(list,capacity,count,1);}
wgn_vlan_rule *wgn_vlan_list_get_all_from_nvram(wgn_vlan_rule *list,size_t capacity,size_t *count){return read_vlan_settings(list,capacity,count,0);}
wgn_vlan_rule *wgn_vlan_list_get_other_from_nvram(wgn_vlan_rule *list,size_t capacity,size_t *count){return read_vlan_settings(list,capacity,count,2);}
void wgn_vlan_list_set_to_nvram(wgn_vlan_rule *list,size_t count){
 if(!list||!count)return;
 if(count>8){errno=ENOSPC;return;}
 wgn_vlan_rule *merged=malloc(8*sizeof(*merged));if(!merged)return;
 char *buffer=malloc(8193);if(!buffer){free(merged);return;}
 memset(merged,0,8*sizeof(*merged));size_t retained=0;
 if(!wgn_vlan_list_get_other_from_nvram(merged,8,&retained)){free(merged);free(buffer);return;}
 if(count>8-retained){free(merged);free(buffer);errno=ENOSPC;return;}
 for(size_t i=0;i<count;i++)if(list[i].type==1)memcpy(&merged[retained+i],&list[i],sizeof(*list));
 memset(buffer,0,8193);
 if(wgn_vlan_list_set_to_buffer(merged,retained+count,buffer,8192)){nvram_set("vlan_rulelist",buffer);nvram_commit();}
 free(merged);free(buffer);
}

/* guest-ifnames.c */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
extern wgn_vlan_rule *wgn_vlan_list_get_from_nvram(wgn_vlan_rule *,size_t,size_t *);
extern void wgn_vlan_list_wl_subunit_shift(wgn_vlan_rule *,size_t);
extern void wgn_get_wl_unit(int *,int *,wgn_vlan_rule *);
extern int nvram_get_int(const char *);
char *wgn_guest_ifnames(int band,int total,char *buffer,size_t capacity){
 if(band<0||!buffer||!capacity)return NULL;
 wgn_vlan_rule *list=malloc(8*sizeof(*list));if(!list)return NULL;
 memset(buffer,0,capacity);memset(list,0,8*sizeof(*list));size_t count=0;
 if(!wgn_vlan_list_get_from_nvram(list,8,&count)){free(list);return NULL;}
 if(nvram_get_int("re_mode")==1)wgn_vlan_list_wl_subunit_shift(list,count);
 if(total<=0||(size_t)total>count)total=(int)count;
 size_t requested=0;int emitted=0;
 for(size_t i=0;i<count&&requested<capacity&&emitted<total;i++){
  int unit,sub;wgn_get_wl_unit(&unit,&sub,&list[i]);if(unit!=band||sub<=0)continue;
  char name[33];snprintf(name,sizeof(name),"wl%d.%d ",band,sub);
  size_t used=strlen(buffer),n=strlen(name),copied=n;
  if(copied>capacity-used-1)copied=capacity-used-1;
  memcpy(buffer+used,name,copied);buffer[used+copied]=0;
  requested+=n;emitted++;
 }
 if(*buffer)buffer[strlen(buffer)-1]=0;
 free(list);return requested&&requested<=capacity?buffer:NULL;
}
char *wgn_guest_all_ifnames(char *buffer,size_t capacity){
 if(!buffer||!capacity)return NULL;
 wgn_vlan_rule *list=malloc(8*sizeof(*list));if(!list)return NULL;
 memset(list,0,8*sizeof(*list));size_t count=0;
 if(!wgn_vlan_list_get_from_nvram(list,8,&count)){free(list);return NULL;}
 if(nvram_get_int("re_mode")==1)wgn_vlan_list_wl_subunit_shift(list,count);
 size_t requested=0;
 for(size_t i=0;i<count&&requested<capacity;i++){
  int unit,sub;wgn_get_wl_unit(&unit,&sub,&list[i]);if(unit<0||sub<=0)continue;
  char name[33];snprintf(name,sizeof(name),"wl%d.%d ",unit,sub);
  size_t used=strnlen(buffer,capacity),n=strlen(name);
  if(used<capacity){size_t copied=n;if(copied>capacity-used-1)copied=capacity-used-1;memcpy(buffer+used,name,copied);buffer[used+copied]=0;}
  requested+=n;
 }
 if(*buffer)buffer[strlen(buffer)-1]=0;
 free(list);return requested&&requested<=capacity?buffer:NULL;
}
int wgn_guest_is_enabled(void){
 char names[1024]={0};if(!wgn_guest_all_ifnames(names,1023))return 0;
 char *p=names;
 while(1){
  p+=strspn(p," ");if(!*p)return 0;
  char word[64],key[64];size_t n=strcspn(p," ");if(n>63)n=63;memcpy(word,p,n);word[n]=0;
  snprintf(key,sizeof(key),"%.*s_bss_enabled",51,word);
  if(nvram_get_int(key))return 1;
  char *next=strchr(p,' ');if(!next)return 0;p=next;
 }
}
char *wgn_guest_vlans(char *ifnames,char *buffer,size_t capacity){
 if(!ifnames||!buffer||!capacity)return NULL;
 wgn_vlan_rule *list=malloc(8*sizeof(*list));if(!list)return NULL;
 memset(buffer,0,capacity);memset(list,0,8*sizeof(*list));size_t count=0;
 if(!wgn_vlan_list_get_from_nvram(list,8,&count)){free(list);return NULL;}
 size_t last_length=0;const char *p=ifnames;
 while(count){
  p+=strspn(p," ");if(!*p)break;
  char word[64];size_t n=strcspn(p," ");if(n>63)n=63;memcpy(word,p,n);word[n]=0;
  int unit=-1,sub=-1;sscanf(word,"wl%d.%d_%*s",&unit,&sub);
  if(unit>=0&&sub>0){
   last_length=0;
   for(size_t i=0;i<count;i++){
    int u,s;wgn_get_wl_unit(&u,&s,&list[i]);if(u!=unit||s!=sub)continue;
    char text[33];int32_t vid;memcpy(&vid,&list[i].vid,sizeof(vid));snprintf(text,sizeof(text),"%d ",vid);
    size_t used=strlen(buffer),copied=strlen(text);last_length=copied;
    if(copied>capacity-used-1)copied=capacity-used-1;
    memcpy(buffer+used,text,copied);buffer[used+copied]=0;break;
   }
  }
  const char *next=strchr(p,' ');if(!next)break;p=next;
 }
 if(*buffer)buffer[strlen(buffer)-1]=0;
 free(list);return last_length&&last_length<=capacity?buffer:NULL;
}
extern wgn_vlan_rule *wgn_vlan_list_find_unit(wgn_vlan_rule *,size_t,int,int);
void wgn_subnet(const char *ifname,char *net,int length){
 if(!ifname||!net||length<=0)return;
 int unit=-1,sub=-1;sscanf(ifname,"wl%d.%d_",&unit,&sub);if(unit<0||sub<=0)return;
 wgn_vlan_rule *list=malloc(8*sizeof(*list));if(!list)return;
 memset(list,0,8*sizeof(*list));size_t count=0;
 if(wgn_vlan_list_get_from_nvram(list,8,&count)){
  wgn_vlan_rule *match=wgn_vlan_list_find_unit(list,count,unit,sub);
  if(match)snprintf(net,(size_t)length,"%s",match->subnet_name);
 }
 free(list);
}

/* check-settings.c */
#include <stdlib.h>
#include <string.h>
extern char *nvram_default_get(const char *),*nvram_get(const char *);
extern int nvram_set(const char *,const char *),nvram_commit(void);
void wgn_check_settings(void){
 const char *keys[]={"vlan_rulelist","subnet_rulelist"};
 for(unsigned i=0;i<2;i++){
  const char *defaults=nvram_default_get(keys[i]);if(!defaults)continue;
  const char *value=nvram_get(keys[i]);char *copy=strdup(value?value:"");
  if(!copy)continue;
  int different=strncmp(defaults,copy,strlen(defaults));free(copy);
  if(different){nvram_set(keys[i],defaults);nvram_commit();}
 }
}

/* vlan-validation.c */
#include <stdio.h>
#include <string.h>
extern int cprintf(const char *,...);
int wgn_check_vlan_invalid(char *word,char *iface){
 if(!word||!iface)return 0;
 FILE *f=fopen64("/proc/net/vlan/config","r");
 if(!f){cprintf("FAIL to open vlan config\n");return 0;}
 char line[1024];int found=0;
 char *header=fgets(line,sizeof(line),f);header=fgets(line,sizeof(line),f);(void)header;
 while(fgets(line,sizeof(line),f)){
  char vlan[20]={0},parent[20]={0};
  if(sscanf(line,"%19s %*s %*s %*s %19s",vlan,parent)!=2)continue;
  if(strcmp(vlan,word))continue;
  cprintf("%s is a Vlan and we will use other names..\n",word);strcpy(iface,parent);found=1;break;
 }
 fclose(f);return found;
}
