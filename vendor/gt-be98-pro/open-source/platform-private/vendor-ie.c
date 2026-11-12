#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern char *nvram_get(const char *);
extern int wl_ioctl(char *,int,void *,int);
extern int cprintf(const char *,...);
static int operation(int unit,int sub,const char *command,uint32_t flag,int length,unsigned char *oui,unsigned char *data)
{
    if(flag&~127u){cprintf("Invalid packet flag 0x%x (%d)\n",flag,flag);return -1;}
    if(length>255){cprintf("IE length is %d, should be <= %d\n",length,255);return -1;}
    if(length<3){cprintf("IE length is %d, should be >= %d\n",length,3);return -1;}
    if(!oui||(!data&&length>3))return -1;
    size_t size=(size_t)length+20;
    unsigned char *request=malloc(size);
    if(!request){cprintf("memory alloc failure\n");return -1;}
    memset(request,0,size);memcpy(request,command,3);
    uint32_t count=1;memcpy(request+4,&count,4);memcpy(request+8,&flag,4);
    request[18]=221;request[19]=(unsigned char)length;memcpy(request+20,oui,3);
    if(length>3)memcpy(request+23,data,(size_t)length-3);
    unsigned char buffer[16384]={0};memcpy(buffer,"vndr_ie",8);memcpy(buffer+8,request,size);
    char prefix[14],full[32],key[100],ifname[16];
    if(sub>0)snprintf(full,sizeof(full),"wl%d.%d_",unit,sub);
    else snprintf(full,sizeof(full),"wl%d_",unit);
    snprintf(prefix,sizeof(prefix),"%.13s",full);
    snprintf(key,sizeof(key),"%smode",prefix);
    char *mode=nvram_get(key);int ap=mode&&!strcmp(mode,"ap");
    if(sub>0&&!ap){free(request);return 0;}
    if(sub<=0&&!ap)snprintf(prefix,sizeof(prefix),"wl%d.1_",unit);
    snprintf(key,sizeof(key),"%sifname",prefix);
    char *name=nvram_get(key);snprintf(ifname,sizeof(ifname),"%s",name?name:"");
    int result=wl_ioctl(ifname,263,buffer,sizeof(buffer));free(request);return result;
}
int wl_add_ie(int unit,int sub,uint32_t flag,int length,unsigned char *oui,unsigned char *data)
{int rc=operation(unit,sub,"add",flag,length,oui,data);if(rc)cprintf("error adding IE: %d\n",rc);return rc;}
int wl_del_ie(int unit,int sub,uint32_t flag,int length,unsigned char *oui,unsigned char *data)
{int rc=operation(unit,sub,"del",flag,length,oui,data);if(rc)cprintf("error deleting IE: %d\n",rc);return rc;}
