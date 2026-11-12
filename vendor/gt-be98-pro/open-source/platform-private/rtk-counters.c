#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
extern int f_read_string(const char *,char *,int);
/* GT-BE98 Pro request/response: 101 32-bit words, as in rtk_stat_port_cntr_t. */
uint64_t rtk_get_phy_mib(int port,char *type)
{
    if(!type)return 0;
    if(port==0){char path[100],value[32]={0};snprintf(path,sizeof(path),"/sys/class/net/eth0/statistics/%s",type);f_read_string(path,value,sizeof(value));return strtoull(value,NULL,10);}
    if(port<1||port>5)return 0;
    uint32_t words[101]={0}, result=0;
    int fd=open("/dev/rtkswitch",O_RDONLY);
    if(fd<0){perror("/dev/rtkswitch");return 0;}
    words[0]=(uint32_t)port-1;
    if(ioctl(fd,1,words)<0){perror("rtkswitch ioctl");close(fd);return 0;}
    if(!strcmp(type,"tx_bytes"))result=words[3];
    else if(!strcmp(type,"rx_bytes"))result=words[1];
    else if(!strcmp(type,"tx_packets"))result=words[11]+words[13]+words[15];
    else if(!strcmp(type,"rx_packets"))result=words[5]+words[7];
    else if(!strcmp(type,"rx_crc_errors"))result=words[30]+words[31];
    close(fd);
    /* Preserve the existing 32-bit accumulator and signed widening ABI. */
    return (result&UINT32_C(0x80000000))?(UINT64_C(0xffffffff00000000)|result):result;
}
