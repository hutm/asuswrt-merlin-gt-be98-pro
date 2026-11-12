#include <stdio.h>
#include <string.h>
extern char *nvram_get(const char *);
extern int nvram_set(const char *,const char *);
extern int wl_ioctl(char *,int,void *,int);
extern int cprintf(const char *,...);
int avbl_reset_exclvalid(char *ifname)
{
    if(!ifname)return -1;
    int unit=0;
    if(wl_ioctl(ifname,14,&unit,sizeof(unit))){cprintf("%s get wl instance failed\n",ifname);return -1;}
    char channel[64],source[64],dest[64];
    snprintf(channel,sizeof(channel),"wl%d_chanspec",unit);
    char *value=nvram_get(channel);
    if(!value||strcmp(value,"0"))return -1;
    snprintf(source,sizeof(source),"wl%d_acs_excl_chans",unit);
    snprintf(dest,sizeof(dest),"wl%d_acs_excl_chans_valid",unit);
    value=nvram_get(source);nvram_set(dest,value?value:"");
    value=nvram_get(dest);
    cprintf("\n%s set exclchans valid[%d]:[%s]=[%s]\n","avbl_reset_exclvalid",unit,dest,value?value:"");
    return 0;
}
int chk_acscli2_cmds(char *command)
{
    if(!command)return 0;
    char key[32],line[64]={0};int found=0;
    snprintf(key,sizeof(key),"acscli2_%s",command);
    FILE *stream=popen("acs_cli2 -C","r");
    if(stream){
        while(fgets(line,sizeof(line),stream))if(strstr(line,command)){found=1;break;}
        pclose(stream);
    }
    nvram_set(key,found?"1":"0");return found;
}
