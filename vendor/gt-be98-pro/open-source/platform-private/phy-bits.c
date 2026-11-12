#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <bcmnet.h>
extern struct ethctl_data ethctl;
extern int cprintf(const char *,...);
extern int nvram_get_int(const char *);
int extphy_bit_op(unsigned int reg,unsigned int val,int write,unsigned int start,unsigned int end,unsigned int wait_ms)
{
    int fd=socket(AF_INET,SOCK_DGRAM,0);
    if(fd<0){fputs("socket open error\n",stderr);return -1;}
    struct ifreq ifr={0};strcpy(ifr.ifr_name,"bcmsw");
    if(ioctl(fd,0x8933,&ifr)<0){fprintf(stderr,"ioctl failed. check if %s exists\n",ifr.ifr_name);close(fd);return -1;}
    ethctl.phy_addr=nvram_get_int("ext_phy_model")==2?25:nvram_get_int("ext_phy_model")==1?3:30;
    ethctl.op=5;ethctl.phy_reg=reg;ethctl.flags=1;ifr.ifr_data=(void*)&ethctl;
    if(ioctl(fd,0x89f9,&ifr)||ethctl.ret_val){cprintf("GET ERROR!!!\n");fputs("command return error!\n",stderr);close(fd);return -1;}
    if(start>=16)start=0;
    if(end>=15)end=15;
    uint32_t mask=0;
    for(unsigned i=start;i<=end;i++)mask|=UINT32_C(1)<<i;
    uint32_t original=(uint32_t)ethctl.val;
    if(!write){
        ethctl.val=(mask&original)>>start;
        cprintf("[GET] 0x%08x [%u:%u] = 0x%04x, full 0x%04x\n",reg,end,start,(unsigned)ethctl.val,original);
        close(fd);return ethctl.val;
    }
    cprintf("[Ori] 0x%08x [%u:%u] = 0x%04x\n",reg,15u,0u,original);
    ethctl.op=6;
    ethctl.val=(int)(((mask^UINT32_C(0xffff))&original)+(val<<start));
    if(ioctl(fd,0x89f9,&ifr)||ethctl.ret_val){cprintf("SET ERROR!!!\n");fputs("command return error!\n",stderr);close(fd);return -1;}
    cprintf("[SET] 0x%08x [%u:%u] = 0x%04x, full 0x%04x\n",reg,end,start,val,(unsigned)ethctl.val);
    close(fd);
    if(wait_ms)usleep((uint32_t)(1000u*wait_ms));
    return ethctl.val;
}
