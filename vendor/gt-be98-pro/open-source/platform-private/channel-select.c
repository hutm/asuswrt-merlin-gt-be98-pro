#include <stdint.h>
#include <string.h>
extern int wl_iovar_getbuf(char *,const char *,void *,int,void *,int);
extern unsigned char wf_chspec_primary20_chan(uint16_t);
extern void dbg(const char *,...);
static int load(char *name,uint32_t list[4096])
{
    uint16_t filter=0;memset(list,0,16384);
    if(wl_iovar_getbuf(name,"chanspecs",&filter,2,list,16384)<0){dbg("failed to get valid chanspec list\n");return 0;}
    int32_t count;memcpy(&count,list,4);
    if(!count)dbg("number of valid chanspec is 0\n");
    if(count<0||count>4095)return 0;
    return count;
}
int with_non_dfs_chspec(char *name)
{
    uint32_t list[4096];int count=load(name,list);
    for(int i=0;i<count;i++){
        uint16_t spec=(uint16_t)list[i+1];
        if(wf_chspec_primary20_chan(spec)<=48||wf_chspec_primary20_chan(spec)>148)return 1;
    }
    return 0;
}
static uint16_t same_bandwidth(char *name,uint16_t current,int high)
{
    uint32_t list[4096];int count=load(name,list);
    for(int i=0;i<count;i++){
        uint16_t spec=(uint16_t)list[i+1];unsigned channel=wf_chspec_primary20_chan(spec);
        if((high?channel>148:channel<=48)&&!((spec^current)&0x3800))return spec;
    }
    return 0;
}
uint16_t select_band1_chspec_with_same_bw(char *name,uint16_t current){return same_bandwidth(name,current,0);}
uint16_t select_band4_chspec_with_same_bw(char *name,uint16_t current){return same_bandwidth(name,current,1);}
uint16_t select_chspec_with_band_bw(char *name,int band,int bandwidth,uint16_t current)
{
    uint32_t list[4096];int count=load(name,list);
    for(int i=0;i<count;i++){
        uint16_t spec=(uint16_t)list[i+1];
        int candidate_band=wf_chspec_primary20_chan(spec)<=51?1:
            wf_chspec_primary20_chan(spec)<=99?2:
            wf_chspec_primary20_chan(spec)<=148?3:
            wf_chspec_primary20_chan(spec)<=168?4:5;
        if(candidate_band!=band)continue;
        unsigned desired=bandwidth==0?(current&0x3800):bandwidth==1?0x1000:bandwidth==2?0x1800:bandwidth==3?0x2000:0x2800;
        if((spec&0x3800)==desired)return spec;
    }
    return 0;
}
