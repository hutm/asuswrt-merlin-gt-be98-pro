#include <stdint.h>
extern int bcm_reg_read_X(int unit,unsigned int address,void *value,int length);
extern int bcm_reg_write_X(int unit,unsigned int address,void *value,int length);
extern int bcm_pseudo_mdio_read(unsigned int address,void *value,int length);
extern int bcm_pseudo_mdio_write(unsigned int address,void *value,int length);
uint64_t hnd_ethswctl(int action,unsigned int address,int length,int write,uint64_t data)
{
    uint64_t value=0;
    int result;
    if(action==0){
        if(!write){bcm_reg_read_X(1,address,&value,length);return value;}
        value=data;
        result=bcm_reg_write_X(1,address,&value,length);
    }else if(action==1){
        if(!write){bcm_pseudo_mdio_read(address,&value,length);return value;}
        value=data;
        result=bcm_pseudo_mdio_write(address,&value,length);
    }else return 0;
    return (uint64_t)(int64_t)result;
}
