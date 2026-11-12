/* SPDX-License-Identifier: GPL-2.0-only */
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <limits.h>
#include <unistd.h>

/* Private request ABI used by the GT-BE98 Pro reference driver. */
enum {
    QUERY_PORTS = 0x89f4,
    ETHCTL_REQUEST = 0x89f9,
    SWITCH_REQUEST = 0x89fa,
    MII_PHY = 0x8947,
    MII_READ = 0x8948,
    MII_WRITE = 0x8949,
    ETHCTL_WORDS = 58,
    SWITCH_WORDS = 267,
    SW_OPERATION = 0,
    SW_UNIT = 4,
    SW_PORT = 5,
    SW_FLAGS = 6,
    SW_TYPE = 7,
    SW_SPEED = 8,
    SW_DUPLEX = 9,
    SW_PHY_CONFIG = 58
};
static uint32_t request[ETHCTL_WORDS];
static int send_request(int fd,struct ifreq *ifr,unsigned op){
 request[0]=op;ifr->ifr_data=(void *)request;
 int rc=ioctl(fd,ETHCTL_REQUEST,ifr);if(rc)fputs("command return error!\n",stderr);return rc;
}
static int command_stats(int fd,struct ifreq *ifr,int argc,char **argv){(void)argc;(void)argv;return send_request(fd,ifr,4);}
static int command_power(int fd,struct ifreq *ifr,int argc,char **argv){
 (void)argc;if(!argv[3])return 1;unsigned op;
 if(!strcmp(argv[3],"up"))op=18;else if(!strcmp(argv[3],"down"))op=19;else return 1;
 int rc=send_request(fd,ifr,op);if(!rc)printf("Powered %s \n",argv[3]);return rc;
}
static int command_vport(int fd,struct ifreq *ifr,int argc,char **argv){
 (void)fd;(void)argc;static unsigned query_result;
 if(!argv[3])return 1;
 if(!strcmp(argv[3],"enable")||!strcmp(argv[3],"disable"))return 0;
 if(strcmp(argv[3],"query"))return 1;
 query_result=0;ifr->ifr_data=(void *)&query_result;printf("%u\n",query_result);return 0;
}

static int32_t signed_number(const char *s){long long n=strtoll(s,NULL,0);if(n>INT32_MAX){errno=ERANGE;return INT32_MAX;}if(n<INT32_MIN){errno=ERANGE;return INT32_MIN;}return n;}
static uint32_t unsigned_number(const char *s){while(isspace((unsigned char)*s))s++;int negative=*s=='-';if(negative){s++;if(!isdigit((unsigned char)*s))return 0;}unsigned long long n=strtoull(s,NULL,0);if(n>UINT32_MAX){errno=ERANGE;return UINT32_MAX;}return negative?0u-(uint32_t)n:(uint32_t)n;}
static void put(struct ifreq *ifr,unsigned index,unsigned value){uint16_t n=value;memcpy((char *)&ifr->ifr_ifru+index*2,&n,2);}
static unsigned get(struct ifreq *ifr,unsigned index){uint16_t n;memcpy(&n,(char *)&ifr->ifr_ifru+index*2,2);return n;}
static unsigned arm_bit(uint32_t n,int bit){unsigned shift=(unsigned)bit&255;return shift<32?(n>>shift)&1:0;}
static int phyid(int fd,struct ifreq *ifr,int port){
 int map=0;ifr->ifr_data=(void *)&map;
 if(ioctl(fd,QUERY_PORTS,ifr)<0){fprintf(stderr,"Error: Interface %s ioctl SIOCGQUERYNUMPORTS error!\n",ifr->ifr_name);return -1;}
 if(map<0)return -1;
 if(map){
  if(port==-1){unsigned bits=__builtin_popcount((unsigned)map);if(bits>1){fprintf(stderr,"Error: Interface %s has sub ports, please specified one of port map: 0x%x\n",ifr->ifr_name,map);return -1;}port=__builtin_ctz((unsigned)map);}
  if(!arm_bit(map,port)){fprintf(stderr,"Specified SubPort %d is not interface %s's member port with map %x\n",port,ifr->ifr_name,map);return -1;}
 }else if(port!=-1){fprintf(stderr,"Interface %s has no sub port\n",ifr->ifr_name);return -1;}
 put(ifr,2,port);if(ioctl(fd,MII_PHY,ifr)<0)return -1;return get(ifr,0)|(get(ifr,3)<<16);
}
static unsigned read_mii(int fd,struct ifreq *ifr,uint32_t phy,unsigned reg){
 put(ifr,1,reg);put(ifr,3,phy>>16);put(ifr,0,phy&31);
 if(ioctl(fd,MII_READ,ifr)>=0)return get(ifr,3);
 fprintf(stderr,"SIOCGMIIREG on %s failed: %s\n",ifr->ifr_name,strerror(errno));return 0;
}
static void write_mii(int fd,struct ifreq *ifr,uint32_t phy,unsigned reg,unsigned value){
 put(ifr,1,reg);put(ifr,2,value);put(ifr,3,phy>>16);put(ifr,0,phy&31);
 if(ioctl(fd,MII_WRITE,ifr)<0)fprintf(stderr,"SIOCSMIIREG on %s failed: %s\n",ifr->ifr_name,strerror(errno));
}
static int command_reg(int fd,struct ifreq *ifr,int argc,char **argv){
 (void)argc;int port=-1;unsigned reg=UINT32_MAX,value=0;int write=0;
 for(char **arg=argv+3;*arg;arg++){
  if(!strcmp(*arg,"port")){if(!arg[1])return -1;port=signed_number(*++arg);continue;}
  unsigned n=unsigned_number(*arg);if(reg==UINT32_MAX){reg=n;if(reg>31)return -1;}else{value=n;write=1;}
 }
 int phy=phyid(fd,ifr,port);if(phy==-1)return -1;
 if(write)write_mii(fd,ifr,phy,reg,value);
 unsigned result=read_mii(fd,ifr,phy,reg);printf("mii (phy addr 0x%x) register %d is 0x%04x\n",phy,reg,result);return result;
}

static int command_wirespeed(int fd,struct ifreq *ifr,int argc,char **argv){
 (void)argc;int port=-1;
 if(argv[4]&&!strcmp(argv[4],"port")){if(!argv[5])return 1;port=signed_number(argv[5]);}
 int phy=phyid(fd,ifr,port);if(phy==-1)return 1;
 if(phy&0x100000){fputs("ethernet@wirespeed is not supported on SERDES interface\n",stderr);return 1;}
 if(!(read_mii(fd,ifr,phy,9)&0x300)){fputs("ethernet@wirespeed is not supported on 10/100Mbps.\n",stderr);return 1;}
 write_mii(fd,ifr,phy,24,0x7007);unsigned value=read_mii(fd,ifr,phy,24);
 if(!argv[3])return 1;
 if(!strcmp(argv[3],"enable")){
  value|=0x8010;write_mii(fd,ifr,phy,24,value);
  read_mii(fd,ifr,phy,0);write_mii(fd,ifr,phy,0,0x1200);
 }else if(!strcmp(argv[3],"disable")){
  value=(value&0x7fef)|0x8000;write_mii(fd,ifr,phy,24,value);
 }
 fputs(value&16?"ethernet@wirespeed is enabled\n":"ethernet@wirespeed is disabled\n",stderr);return 1;
}



static const char *speed(unsigned v){return v&0x1000?"Auto Detection":v&0x80?"2.5Gps":v&0x40?"1Gps":v&0x2000?"100Mbps":"10Mbps";}
static void show_speed(int fd,struct ifreq *ifr,unsigned phy,int port){
 if(phy&0x100000){
  uint32_t data[SWITCH_WORDS]={0};data[SW_OPERATION]=26;data[SW_FLAGS]=1;data[SW_TYPE]=2;if(port!=-1){data[SW_UNIT]=-1;data[SW_PORT]=port;data[SW_FLAGS]=3;}ifr->ifr_data=(void *)data;
  int rc=ioctl(fd,SWITCH_REQUEST,ifr);if(rc){fprintf(stderr,"ioctl command return error %d!\n",rc);return;}
  printf(" Serdes is Configured at Speed: %s\n",speed(data[SW_PHY_CONFIG]));if(!data[SW_SPEED])puts("  Link is down.");else printf("  Link is up at %s, %s duplex\n",speed(data[SW_SPEED]),data[SW_DUPLEX]?"Full":"Half");return;
 }
 unsigned control=read_mii(fd,ifr,phy,0),status=read_mii(fd,ifr,phy,1);
 if(!status||control==0xffff){fputs("  No MII transceiver present!.\n",stderr);return;}
 unsigned advertised=read_mii(fd,ifr,phy,4),partner=read_mii(fd,ifr,phy,5);
 if(control&0x1000){
  puts("Auto-negotiation enabled.");unsigned gig=read_mii(fd,ifr,phy,9);
  if(gig&0x300){
   write_mii(fd,ifr,phy,24,0x7007);unsigned shadow=read_mii(fd,ifr,phy,24);
   if(shadow&16){unsigned result=read_mii(fd,ifr,phy,25)&0x700;
    const char *message=result==0x700?"The autonegotiated media type is 1000BT Full Duplex \n":result==0x600?"The autonegotiated media type is 1000BT Half Duplex \n":result==0x500?"The autonegotiated media type is 100BT Full Duplex \n":result==0x300?"The autonegotiated media type is 100BT Half Duplex \n":result==0x200?"The autonegotiated media type is 10BT Full Duplex \n":result==0x100?"The autonegotiated media type is 10BT Half Duplex \n":NULL;
    if(message)fputs(message,stderr);
    return;
   }
  }
  unsigned peer_gig=read_mii(fd,ifr,phy,10);
  if((gig&0x200)&&(peer_gig&0x800))puts("The autonegotiated media type is 1000BT Full Duplex ");
  else if((gig&0x100)&&(peer_gig&0x400))puts("The autonegotiated media type is 1000BT Half Duplex ");
  else if(partner&0x4000){
   unsigned common=advertised&partner&0x3e0;static const unsigned order[]={8,9,7,6,5};
   static const char *names[]={"10baseT","10baseT-FD","100baseTx","100baseTx-FD","100baseT4"};int found=0;
   for(unsigned i=0;i<5;i++)if(common&(1u<<order[i])){printf("The autonegotiated media type is %s.\n",names[order[i]-5]);found=1;break;}
   if(!found)fputs("No common media type was autonegotiated!\nThis is extremely unusual and typically indicates a configuration error.\nPerhaps the advertised capability set was intentionally limited.\n",stderr);
  }
 }else printf("Auto-negotiation disabled, with\n Speed fixed at %s, %s-duplex.\n",speed(control),control&0x100?"full":"half");
 read_mii(fd,ifr,phy,1);status=read_mii(fd,ifr,phy,1);printf("Link is %s\n",status&4?"up":"down");
}
static int command_reset(int fd,struct ifreq *ifr,int argc,char **argv){
 (void)argc;int port=-1;if(argv[3]&&!strcmp(argv[3],"port")&&argv[4])port=signed_number(argv[4]);
 int phy=phyid(fd,ifr,port);if(phy==-1)return -1;
 write_mii(fd,ifr,phy,0,0x8000);sleep(2);show_speed(fd,ifr,phy,port);return 0;
}
static int command_media(int fd,struct ifreq *ifr,int argc,char **argv){
 (void)argc;int port=-1,mode=-1;static const char *options[]={"auto","100FD","100HD","10FD","10HD","1000FD","1000HD","2500FD"};
 for(char **arg=argv+3;*arg;arg++){
  if(!strcmp(*arg,"port")){if(!arg[1])return -1;port=signed_number(*++arg);continue;}
  int found=-1;for(int i=0;i<8;i++)if(!strcmp(*arg,options[i])){found=i;break;}
  if(found<0)return -1;
  mode=found;
 }
 int phy=phyid(fd,ifr,port);if(phy==-1)return -1;
 if(mode>=0){
  static const unsigned controls[]={0x1200,0x2100,0x2000,0x100,0,0x1340,0x1240,0x180};unsigned control=controls[mode];
  if(phy&0x100000){
   if(mode==5)control=0x140;
   uint32_t data[SWITCH_WORDS]={0};data[SW_OPERATION]=26;data[SW_FLAGS]=1;data[SW_TYPE]=3;data[SW_PHY_CONFIG]=control;
   if(port!=-1){data[SW_UNIT]=-1;data[SW_PORT]=port;data[SW_FLAGS]=3;}ifr->ifr_data=(void *)data;
   int rc=ioctl(fd,SWITCH_REQUEST,ifr);if(rc){fprintf(stderr,"ioctl command return error %d!\n",rc);return -1;}
  }else{
   unsigned advertisement=read_mii(fd,ifr,phy,4),gig=read_mii(fd,ifr,phy,9)|0x300;
   if(mode==5||mode==6){advertisement&=~0x3e0;gig&=~(mode==5?0x100:0x200);}else advertisement|=0x3e0;
   write_mii(fd,ifr,phy,4,advertisement);write_mii(fd,ifr,phy,9,gig);write_mii(fd,ifr,phy,0,control);
   if(mode==0)sleep(2);
  }
 }
 show_speed(fd,ifr,phy,port);return 0;
}

struct command {int nargs;const char *name;int (*fn)(int,struct ifreq *,int,char **);const char *help;};
static const struct command commands[]={
 {0,"media-type",command_media,": Set/Get media type\n  ethctl <interface> media-type [option] [port <sub_port#> ]\n    [option]: auto - auto select\n              1000FD - 1000Mb, Full Duplex\n              1000HD - 1000Mb, Half Duplex\n              100FD - 100Mb, Full Duplex\n              100HD - 100Mb, Half Duplex\n              10FD  - 10Mb,  Full Duplex\n              10HD  - 10Mb,  Half Duplex\n    [port <sub_port#>]: required if <interface> has Crossbar or Trunk port underneath\n"},
 {0,"phy-reset",command_reset,": Soft reset the transceiver\n  ethctl <interface> phy-reset [port <sub_port#>]\n    [port <sub_port#>]: required if <interface> has Crossbar or Trunk port underneath\n"},
 {1,"reg",command_reg,": Set/Get port mii register\n  ethctl <interface> reg <[0-31]> [0xhhhh] [port <sub_port#>]\n    [port <sub_port#>]: required if <interface> has Crossbar or Trunk port underneath\n"},
 {1,"phy-power",command_power,": Phy power up/down control\n  ethctl <interface> phy-power <up|down>"},
 {1,"vport",command_vport,": Enable/disable/query Switch for VLAN port mapping\n  ethctl <interface> vport <enable|disable|query>"},
 {0,"stats",command_stats,": Display software stats\n  ethctl <interface> stats"},
 {1,"ethernet@wirespeed",command_wirespeed,": Enable/Disable ethernet@wirespeed\n  ethctl <interface> ethernet@wirespeed <show|enable|disable> [port <sub_port#>]\n    [port <sub_port#>]: required if <interface> has Crossbar or Trunk port underneath\n"},
};
const struct command *command_lookup(const char *name){if(!name)return NULL;for(unsigned i=0;i<sizeof(commands)/sizeof(*commands);i++)if(!strcmp(commands[i].name,name))return &commands[i];return NULL;}
