#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stddef.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <bcmnet.h>
extern int cprintf(const char *,...);
struct ethctl_data ethctl;
_Static_assert(sizeof(ethctl)==232,"ethctl public object ABI");
_Static_assert(offsetof(struct ethctl_data,phy_addr)==28,"address");
_Static_assert(offsetof(struct ethctl_data,phy_reg)==32,"register");
_Static_assert(offsetof(struct ethctl_data,flags)==52,"flags");
int ethctl_phy_op(char *type,int address,unsigned int reg,unsigned int value,int write)
{
    struct ifreq ifr={0};
    strcpy(ifr.ifr_name,"bcmsw");
    int fd=socket(AF_INET,SOCK_DGRAM,0);
    if(fd<0){fputs("socket open error\n",stderr);return -1;}
    if(ioctl(fd,0x8933,&ifr)<0){fprintf(stderr,"ioctl failed. check if %s exists\n",ifr.ifr_name);close(fd);return -1;}
    unsigned flags;
    if(!type){close(fd);return -1;}
    if(!strcmp(type,"ext"))flags=1;
    else if(!strcmp(type,"int"))flags=0;
    else if(!strcmp(type,"extsw"))flags=2;
    else if(!strcmp(type,"i2c"))flags=4;
    else if(!strcmp(type,"10gserdes"))flags=128;
    else if(!strcmp(type,"10gpcs"))flags=256;
    else if(!strcmp(type,"serdespower"))flags=32;
    else if(!strcmp(type,"ext32"))flags=65;
    else {fputs("Unknown phy type!\n",stderr);close(fd);return -1;}
    if((unsigned)address>31){fprintf(stderr,"Invalid Phy Address 0x%02x\n",(unsigned)address);close(fd);return -1;}
    if(flags==32&&reg>2){fprintf(stderr,"Invalid Serdes Power Mode%02x\n",reg);close(fd);return -1;}
    ethctl.phy_addr=address;ethctl.phy_reg=reg;ethctl.flags=flags;ethctl.op=write?6:5;
    if(write)ethctl.val=value;
    ifr.ifr_data=(void*)&ethctl;
    int rc=ioctl(fd,0x89f9,&ifr);
    if(rc||ethctl.ret_val){cprintf(write?"SET ERROR!!!\n":"GET ERROR!!!\n");fputs("command return error!\n",stderr);close(fd);return -1;}
    close(fd);
    return write?0:ethctl.val;
}
