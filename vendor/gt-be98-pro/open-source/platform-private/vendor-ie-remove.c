#include <stdint.h>
#include <stdio.h>
#include <string.h>
extern char *nvram_get(const char *);
extern int wl_ioctl(char *,int,void *,int);
extern int wl_del_ie(int,int,uint32_t,int,unsigned char*,unsigned char*);
extern int cprintf(const char *,...);
void wl_del_ie_with_oui(int unit,int sub,unsigned char *oui)
{
    if(!oui)return;
    unsigned char buffer[16384]={0};
    memcpy(buffer,"vndr_ie",8);
    uint32_t all=UINT32_MAX;memcpy(buffer+8,&all,4);buffer[12]=221;
    char prefix[14],full[32],key[100],ifname[16];
    if(sub>0)snprintf(full,sizeof(full),"wl%d.%d_",unit,sub);
    else snprintf(full,sizeof(full),"wl%d_",unit);
    snprintf(prefix,sizeof(prefix),"%.13s",full);
    snprintf(key,sizeof(key),"%smode",prefix);
    char *mode=nvram_get(key);int ap=mode&&!strcmp(mode,"ap");
    if(sub>0&&!ap)return;
    if(sub<=0&&!ap){snprintf(full,sizeof(full),"wl%d.1_",unit);snprintf(prefix,sizeof(prefix),"%.13s",full);}
    snprintf(key,sizeof(key),"%sifname",prefix);
    char *name=nvram_get(key);snprintf(ifname,sizeof(ifname),"%s",name?name:"");
    int rc=wl_ioctl(ifname,262,buffer,sizeof(buffer));
    if(rc){cprintf("error getting IE: %d\n",rc);return;}
    int32_t count;memcpy(&count,buffer,4);
    size_t offset=4;
    for(int32_t i=0;i<count;i++){
        if(sizeof(buffer)-offset<15)return;
        unsigned char *entry=buffer+offset;unsigned length=entry[11];
        if(length<3||length>sizeof(buffer)-offset-12)return;
        if(!memcmp(entry+12,oui,3)){
            uint32_t flag;memcpy(&flag,entry,4);
            wl_del_ie(unit,sub,flag,(int)length,entry+12,entry+15);
        }
        offset+=12+length;
    }
}
