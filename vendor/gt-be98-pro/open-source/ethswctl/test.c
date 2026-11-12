/* SPDX-License-Identifier: GPL-2.0-only */
#include <stdio.h>
#include <stdint.h>
extern int outputJumboStatus(unsigned,unsigned);
extern int outputJumboStatus_GPHY_0(unsigned);
extern int outputJumboStatus_GPHY_1(unsigned);
extern int outputJumboStatus_GMII_1(unsigned);
extern int outputJumboStatus_GMII_2(unsigned);
extern int outputJumboStatus_GPON_SERDES(unsigned);
extern int outputJumboStatus_MOCA(unsigned);
extern int outputJumboStatus_USB(unsigned);
extern int outputJumboStatus_GPON(unsigned);
extern int outputJumboStatus_MIPS(unsigned);
int main(void){
for(unsigned mask=0;mask<512;mask++){
printf("RETURN %d\n",outputJumboStatus_GPHY_0(mask));
printf("RETURN %d\n",outputJumboStatus_GPHY_1(mask));
printf("RETURN %d\n",outputJumboStatus_GMII_1(mask));
printf("RETURN %d\n",outputJumboStatus_GMII_2(mask));
printf("RETURN %d\n",outputJumboStatus_GPON_SERDES(mask));
printf("RETURN %d\n",outputJumboStatus_MOCA(mask));
printf("RETURN %d\n",outputJumboStatus_USB(mask));
printf("RETURN %d\n",outputJumboStatus_GPON(mask));
printf("RETURN %d\n",outputJumboStatus_MIPS(mask));
for(unsigned port=0;port<1024;port++){printf("CASE %u %u\n",port,mask);printf("RETURN %d\n",outputJumboStatus(port,mask));}
}
unsigned extremes[]={0x80000000u,0xffffffffu,0x12345678u,0xfffffe09u};
for(unsigned i=0;i<4;i++)for(unsigned j=0;j<4;j++){printf("EDGE %u %u\n",i,j);printf("RETURN %d\n",outputJumboStatus(extremes[i],extremes[j]));}
return 0;
}
