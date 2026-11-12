#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
extern int f_read_string(const char *,char *,int);
struct phy_state { uint32_t link[4], speed[4], duplex[4]; };
_Static_assert(sizeof(struct phy_state)==48,"switch query ABI");
static int read_state(struct phy_state *state)
{
    int fd=open("/dev/rtkswitch",O_RDONLY);
    if(fd<0){perror("/dev/rtkswitch");return -1;}
    memset(state,0,sizeof(*state));
    int rc=ioctl(fd,0,state);
    if(rc<0)perror("rtkswitch ioctl");
    close(fd);
    return rc<0?-1:0;
}
uint32_t rtk_get_phy_speed(int port)
{
    if(port==0){char value[32]={0};f_read_string("/sys/class/net/eth0/speed",value,sizeof(value));return (uint32_t)strtoul(value,NULL,10);}
    if(port<1||port>4)return 0;
    struct phy_state state;
    if(read_state(&state)<0||!state.link[port-1])return 0;
    switch(state.speed[port-1]){case 5:return 2500;case 2:return 1000;case 1:return 100;default:return 10;}
}
uint32_t rtk_get_phy_duplex(int port)
{
    if(port==0){char value[32]={0};f_read_string("/sys/class/net/eth0/duplex",value,sizeof(value));return !strncmp(value,"full",4);}
    if(port<1||port>4)return 0;
    struct phy_state state;
    if(read_state(&state)<0)return 0;
    return state.duplex[port-1];
}

#include <syslog.h>
extern int cprintf(const char *, ...);
uint32_t rtk_get_phy_status(int port)
{
    if(port<0||port>4)return 0;
    if(port>0){struct phy_state state;if(read_state(&state)<0)return 0;return state.link[port-1];}
    char value[32]={0};
    f_read_string("/sys/class/net/eth0/operstate",value,sizeof(value));
    if(!strncmp(value,"up",2))return 1;
    if(!strncmp(value,"down",4))return 0;
    cprintf("PID(%d) %s: Temporarily Router cannot get the PHY(eth%d) status...\n",(int)getpid(),__func__,port);
    syslog(LOG_NOTICE,"%s: Temporarily Router cannot get the PHY(eth%d) status...",__func__,port);
    return 1;
}

extern char *nvram_get(const char *);
extern char *nvram_default_get(const char *);
extern int nvram_get_int(const char *);
extern uint32_t hnd_get_phy_status(char *);
int rtk_lan_phy_status(void)
{
    struct phy_state state;
    if(read_state(&state)<0)return 0;
    int mode=nvram_get("sw_mode")?nvram_get_int("sw_mode"):
             atoi(nvram_default_get("sw_mode"));
    if(mode==1 && nvram_get_int("wans_extwan"))
        return state.link[1] || state.link[2] || state.link[3] || hnd_get_phy_status("eth0");
    return state.link[0] || state.link[1] || state.link[2] || state.link[3];
}
