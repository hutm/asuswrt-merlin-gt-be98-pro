#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <net/if.h>
#include <linux/mii.h>
#include <sys/ioctl.h>
/* GT-BE98 Pro private-object ABI; differs from the legacy mdio.c helper. */
int mdio_read(int fd,struct ifreq *ifr,unsigned int phy_id,int location)
{
    struct mii_ioctl_data request;
    memcpy(&request,&ifr->ifr_ifru,sizeof(request));
    request.phy_id=phy_id&31;
    request.reg_num=(uint16_t)location;
    request.val_out=(uint16_t)(phy_id>>16);
    memcpy(&ifr->ifr_ifru,&request,sizeof(request));
    if(ioctl(fd,0x8948,ifr)<0){
        fprintf(stderr,"SIOCGMIIREG on %s failed: %s\n",ifr->ifr_name,strerror(errno));
        return 0;
    }
    memcpy(&request,&ifr->ifr_ifru,sizeof(request));
    return request.val_out;
}
