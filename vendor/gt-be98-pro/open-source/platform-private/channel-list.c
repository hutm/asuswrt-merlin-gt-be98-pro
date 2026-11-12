#include <stdint.h>
#include <stdio.h>
#include <string.h>
extern char *nvram_get(const char *);
extern int wl_ioctl(char *,int,void *,int);
extern int wl_iovar_getbuf(char *,const char *,void *,int,void *,int);
extern unsigned char wf_chspec_primary20_chan(uint16_t);
extern void dbg(const char *,...);
void wl_list_5g_chans(int unit,int band,int war,char *buf,int len,int bandwidth)
{
    if(!buf||len<=0)return;
    memset(buf,0,(size_t)len);
    char key[64],name[16],full[32],prefix[14];
    snprintf(full,sizeof(full),"wl%d_",unit);snprintf(prefix,sizeof(prefix),"%.13s",full);
    snprintf(key,sizeof(key),"%sifname",prefix);
    char *value=nvram_get(key);snprintf(name,sizeof(name),"%s",value?value:"");
    int radio_band=0;
    if(!wl_ioctl(name,141,&radio_band,4)&&radio_band!=1)return;
    uint32_t list[4096]={0};uint16_t filter=0;
    if(wl_iovar_getbuf(name,"chanspecs",&filter,2,list,sizeof(list))<0){dbg("failed to get valid chanspec list\n");return;}
    int32_t count;memcpy(&count,list,4);
    if(!count){dbg("number of valid chanspec is 0\n");return;}
    if(count<0||count>4095)return;
    int first=1;
    for(int i=0;i<count;i++){
        uint16_t spec=(uint16_t)list[i+1];
        int candidate_band=wf_chspec_primary20_chan(spec)<=51?1:wf_chspec_primary20_chan(spec)<=99?2:wf_chspec_primary20_chan(spec)<=148?3:wf_chspec_primary20_chan(spec)<=168?4:5;
        unsigned width=spec&0x3800;
        int cross=!war&&width==0x2800&&((band==1&&candidate_band==2)||(band==2&&candidate_band==1));
        unsigned wanted=bandwidth==1?0x1000:bandwidth==2?0x1800:bandwidth==3?0x2000:0x2800;
        if((candidate_band==band||cross)&&(!bandwidth||width==wanted)){
            char text[16];snprintf(text,sizeof(text),first?"0x%04x":",0x%04x",spec);
            size_t used=strlen(buf);strncat(buf,text,(size_t)len-1-used);first=0;
        }
    }
}
