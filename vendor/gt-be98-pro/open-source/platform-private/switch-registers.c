#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <bcm/bcmswapitypes.h>
static int switch_socket(struct ifreq *ifr)
{
    int fd=socket(AF_INET,SOCK_DGRAM,0);
    if(fd<0){perror("socket open error\n");return -1;}
    strcpy(ifr->ifr_name,"bcmsw");
    if(ioctl(fd,0x8933,ifr)>=0)return fd;
    strcpy(ifr->ifr_name,"eth0");
    if(ioctl(fd,0x8933,ifr)>=0)return fd;
    close(fd);printf("neither bcmsw nor %s exist\n","eth0");return -1;
}
static int register_access(int mdio,int write,int unit,unsigned int address,void *value,int length)
{
    if(!value||length<0||length>8){errno=EINVAL;return -1;}
    struct ifreq ifr={0};
    struct ethswctl_data request={0};
    int fd=switch_socket(&ifr);
    if(fd<0){puts("ethswctl_init failed. ");return fd;}
    request.op=mdio?20:19;request.type=write?3:2;
    request.unit=unit;request.offset=address;request.length=length;
    /* The pseudo-MDIO ABI passes an eight-byte input value even for short writes. */
    if(write)memcpy(request.data,value,mdio?8:(size_t)length);
    ifr.ifr_data=(void*)&request;
    int result=ioctl(fd,0x89fa,&ifr);
    if(result)puts("ioctl command return error!");
    else if(!write)memcpy(value,request.data,length);
    close(fd);return result;
}
int bcm_reg_read_X(int unit,unsigned int addr,void *value,int len){return register_access(0,0,unit,addr,value,len);}
int bcm_reg_write_X(int unit,unsigned int addr,void *value,int len){return register_access(0,1,unit,addr,value,len);}
int bcm_pseudo_mdio_read(unsigned int addr,void *value,int len){return register_access(1,0,0,addr,value,len);}
int bcm_pseudo_mdio_write(unsigned int addr,void *value,int len){return register_access(1,1,0,addr,value,len);}
