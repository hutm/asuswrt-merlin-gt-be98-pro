#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern int nvram_get_int(const char *),nvram_set_int(const char *,int),get_wl_count(void),get_eth_count(void),isMloConnectionMode(void),check_wifi_use(int),is_mlo_map_by_bandindex(int);
extern char *nvram_get(const char *),*wl_nvname(const char *,int,int);
extern void trans_to_amas_bhmode(void),dbg(const char *,...);
extern int cprintf(const char *,...);
static uint32_t shift(uint32_t v,uint32_t n){n&=255;return n<32?v<<n:0;}
static void addbit(int *p,uint32_t v){uint32_t u;memcpy(&u,p,4);u|=v;memcpy(p,&u,4);}
static const char *safe(const char *key){const char *s=nvram_get(key);return s?s:"";}
static const char *word(const char *p,char out[8]){
 p+=strspn(p," ");size_t n=strcspn(p," ");if(n>7)n=7;memcpy(out,p,n);out[n]=0;const char *q=strchr(p,' ');return q?q:"";
}
static unsigned basewifi(int count,uint32_t use){return count<=2?0x30:use==11?0x70:use==13?0xd0:0x50;}
void trans_to_bhmode(int *eth,int *wifi,int *cost,int *rssi,int *mlo_out)
{
 int original=nvram_get_int("amas_ethernet");if(!original)original=3;
 int wc=get_wl_count(),ec=get_eth_count(),eap=nvram_get_int("amas_eap_bhmode");
 int mlo=isMloConnectionMode(),debug=nvram_get_int("amas_dbg");
 *cost=4;*rssi=5;
 int mode=original,ordinal=0;uint32_t mask=UINT32_MAX;
 if(original>=10&&original<=99){mode=original/10;ordinal=original%10;if(mode!=9&&original>39){if(mode==5)mask=8;else if(mode==6)mask=16;else if(mode==7)mask=32;else if(mode==8)mask=64;else{mode=4;mask=4;}}}
 else if(original>=1000){mode=original/10;ordinal=original%10;if(mode==10000)mask=65536;else if(mode==20000)mask=131072;}
 else if(original>=4&&original<=8){if(original!=5)mask=shift(1,(uint32_t)original-2);}
 int selected=-1;
 if(ordinal){const char *p=safe("amas_ethif_type");p+=strspn(safe("amas_ethif_type")," ");char token[8];int seen=0,index=0;
  for(;;index++){const char *next=word(p,token);if(!*token)break;if(((uint32_t)atoi(token)&mask)!=0)seen++;if(seen==ordinal){selected=index;break;}p=next;}
 }
 if(ec<=0&&eap>0){nvram_set_int("amas_eap_bhmode",0);eap=0;}
 int n2=0,n5=0,n6=0;const char *p=safe("wl_ifnames");p+=strspn(safe("wl_ifnames")," ");char token[8];
 for(int unit=0;;unit++){const char *next=word(p,token);if(!*token)break;if(unit<=3&&nvram_get(wl_nvname("nband",unit,0))){char key[32];snprintf(key,sizeof(key),"wl%d_nband",unit);int b=nvram_get_int(key);if(b==2)n2++;else if(b==1)n5++;else if(b==4)n6++;}p=next;}
 if(debug)cprintf("num2g(%d) num5g(%d) num6g(%d)\n",n2,n5,n6);
 char priority[64];snprintf(priority,sizeof(priority),"%s",safe("sta_priority"));
 uint32_t use=0,mlo_use=0;p=priority;
 for(int unit=0;;unit++){
  int band,index,unused,enabled,consumed=0;
  if(sscanf(p," %d%d%d%d%n",&band,&index,&unused,&enabled,&consumed)!=4)break;
  p+=consumed;char key[64];snprintf(key,sizeof(key),"wl%d_nband_type",unit);int type=nvram_get_int(key);
  if(debug)cprintf("unit(%d) nband_type(%d)\n",unit,type);
  if(band!=2&&band!=5&&band!=6)continue;
  if(enabled==1||(mlo&&is_mlo_map_by_bandindex(index))){uint32_t bit;
   if(band==6){type-=n6==1?4:5;if(debug)cprintf("6G band_shift(%d)\n",type);bit=shift(8,(uint32_t)type);}
   else if(band==5){type-=n5==1?1:2;if(debug)cprintf("5G band_shift(%d)\n",type);bit=shift(2,(uint32_t)type);}
   else{if(debug)cprintf("2G band_shift(%d)\n",type);bit=shift(1,(uint32_t)type);}
   use|=bit;if(check_wifi_use(index))mlo_use|=bit;
  }
  if(debug)cprintf(band==6?"6G use_band(0X%02X) wifi_mlo_use(0X%02X)\n":band==5?"5G use_band(0X%02X) wifi_mlo_use(0X%02X)\n":"2G use_band(0X%02X) wifi_mlo_use(0X%02X)\n",use,mlo_use);
 }
 cprintf("use_band(0X%02X) wifi_mlo_use(0X%02X)\n",use,mlo_use);
 if((uint32_t)nvram_get_int("aimesh_alg")<=1)*rssi=1;
 if(mode==-1){*eth=nvram_get_int("amas_eth_bhmode");*wifi=nvram_get_int("amas_wifi_bhmode");*cost=nvram_get_int("amas_costmode");*rssi=nvram_get_int("amas_rssiscoremode");puts("Connection Priority is custom. Don't process transfer amas_ethernet to bhmode settings.");}
 else if(mode==0){*eth=0;*wifi=0;}
 else if(mode==1){*eth=0;*wifi=wc<=2?0x32:use==11?0x72:use==13?0xd4:0x54;}
 else if(mode==3&&eap>0){for(int i=0;i<ec;i++)addbit(eth,shift(1,((uint32_t)i&~3u)+4u+(uint32_t)i));*wifi=0;*cost=1;}
 else if(mode==100||mode==200||mode==300||mode==400){*wifi=(int)basewifi(wc,use);if(mode==100)addbit(wifi,1);else if(mode==200)addbit(wifi,2);else if(mode==300)addbit(wifi,4);else if(wc==3||(wc==4&&use==11))addbit(wifi,4);else if(wc==4)addbit(wifi,8);*eth=0;*cost=1;*rssi=1;}
 else {
  int selected_mode=mode==2||(mode>=4&&mode<=8)||mode==10000||mode==20000;
  for(int i=0;i<ec;i++){
   if(selected_mode&&i==selected)addbit(eth,shift(1,(uint32_t)i));
   addbit(eth,shift(1,((uint32_t)i&~3u)+4u+(uint32_t)i));
  }
  if(selected_mode&&eap>0){*wifi=0;*cost=1;}
  else{
   *wifi=(int)basewifi(wc,use);
   if(mode==3||selected_mode){int k=0;uint32_t both=use&mlo_use;
    if(mlo){if(n2==1){k=1;if(use&1)addbit(wifi,16);if(both&1)addbit(mlo_out,16);}
     if(n5){if(use&2)addbit(wifi,shift(1,(uint32_t)k+4));if(both&2)addbit(mlo_out,shift(1,(uint32_t)k+4));if(n5==2){if(use&4)addbit(wifi,shift(1,(uint32_t)k+5));if(both&4)addbit(mlo_out,shift(1,(uint32_t)k+5));k+=2;}else k++;}
     if(n6){if(use&8)addbit(wifi,shift(1,(uint32_t)k+4));if(both&8)addbit(mlo_out,shift(1,(uint32_t)k+4));if(n6==2){if(use&16)addbit(wifi,shift(1,(uint32_t)k+5));if(both&16)addbit(mlo_out,shift(1,(uint32_t)k+5));k+=2;}else k++;}
     if(!*mlo_out)*mlo_out=*wifi;
    }
    if(mode==3)dbg("k(%d) wifi_mlo_use(0x%2x) *amas_wifi_mlo_bhmode(0x%2x)\n",k,both,*mlo_out);
    else{*cost=1;*rssi=1;}
   }
  }
 }
 trans_to_amas_bhmode();
}
