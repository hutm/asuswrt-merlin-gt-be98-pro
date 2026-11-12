/* SPDX-License-Identifier: GPL-2.0-only */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <typedefs.h>
#include <bcmtlv.h>
int main(void){
 unsigned char data[256];for(int i=0;i<256;i++)data[i]=(unsigned char)(i*137);
 for(unsigned opts=0;opts<8;opts++)for(int n=0;n<256;n++)for(int mode=0;mode<4;mode++){
  unsigned char out[272],expected[272];bcm_xtlvbuf_t buf;
  int hdr=4-!!(opts&2)-!!(opts&4),size=hdr+n;if(opts&1)size=(size+3)&~3;
  int cap=mode==0?size-1:mode==1?size:mode==2?size+1:272;unsigned id=(unsigned)(n*257+1234)&65535;
  memset(out,0x5a,sizeof(out));memset(expected,0x5a,sizeof(expected));
  assert(bcm_xtlv_hdr_size(opts)==hdr);assert(bcm_xtlv_size_for_data(n,opts)==size);assert(!bcm_xtlv_buf_init(&buf,out,cap,opts));
  int ret=bcm_xtlv_put_data(&buf,(uint16)id,data,n);
  if(cap<size){assert(ret<0);assert(!bcm_xtlv_buf_len(&buf));assert(bcm_xtlv_buf_rlen(&buf)==cap);}
  else{
   assert(!ret);int pos=0;expected[pos++]=(unsigned char)id;if(!(opts&2))expected[pos++]=(unsigned char)(id>>8);expected[pos++]=(unsigned char)n;if(!(opts&4))expected[pos++]=0;assert(pos==hdr);memcpy(expected+pos,data,n);
   assert(bcm_xtlv_buf_len(&buf)==size);assert(bcm_xtlv_buf_rlen(&buf)==cap-size);
   const bcm_xtlv_t *tlv=(const bcm_xtlv_t *)out;uint16 type=0,len=0;const uint8 *body=NULL;
   assert(bcm_valid_xtlv(tlv,size,opts));assert(!bcm_valid_xtlv(tlv,size-1,opts));assert(!bcm_valid_xtlv(NULL,size,opts));
   assert(bcm_xtlv_size(tlv,opts)==size);bcm_xtlv_unpack_xtlv(tlv,&type,&len,&body,opts);assert(type==(opts&2?(id&255):id));assert(len==n);assert(body==out+hdr);
  }
  assert(!memcmp(out,expected,sizeof(out)));printf("%u %d %d %d ",opts,n,cap,ret);for(size_t i=0;i<sizeof(out);i++)printf("%02x",out[i]);putchar('\n');
 }
 return 0;
}
