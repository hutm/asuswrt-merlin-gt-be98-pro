/* SPDX-License-Identifier: GPL-2.0-only */
#include <assert.h>
#include <stdio.h>
#include <typedefs.h>
#include <bcmutils.h>
int main(void){
 uint8 data[1025];for(unsigned i=0;i<sizeof(data);i++)data[i]=(uint8)(i*137+19);
 for(unsigned n=0;n<=1024;n++)for(unsigned k=0;k<4;k++){
  uint32 seed=k==0?0:k==1?~(uint32)0:k==2?0x12345678:0xa5a5a5a5;
  unsigned bits=0;for(unsigned i=0;i<n;i++)for(unsigned b=0;b<8;b++)bits+=(data[i]>>b)&1;
  assert(bcm_bitcount(data,n)==bits);
  printf("crc %u %u %u %u %u %u %u\n",n,k,(unsigned)hndcrc8(data,n,(uint8)seed),(unsigned)hndcrc16(data,n,(uint16)seed),(unsigned)hndcrc32(data,n,seed),(unsigned)bcm_ip_cksum(data,n,seed),bits);
 }
 for(unsigned v=0;v<65536;v++)printf("power %u %u\n",v,(unsigned)bcm_mw_to_qdbm((uint16)v));
 for(unsigned v=0;v<256;v++)printf("qdbm %u %u\n",v,(unsigned)bcm_qdbm_to_mw((uint8)v));
 for(unsigned v=0;v<65536;v++){uint32 r=sqrt_int(v);printf("sqrt %u %u\n",v,(unsigned)r);}
 return 0;
}
