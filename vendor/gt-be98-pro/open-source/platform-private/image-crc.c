#include <stdint.h>
#include <stdio.h>
#include <string.h>
extern uint32_t hndcrc32(unsigned char *,unsigned int,uint32_t);
extern void dbg(const char *,...);
uint32_t img_crc_hnd(char *filename)
{
    if(!filename)return 0;
    FILE *stream=fopen(filename,"r");
    if(!stream){dbg("failed on open file: %s\n",filename);return 0;}
    unsigned char buffer[102420];size_t pending=0;uint32_t crc=UINT32_MAX;
    for(;;){
        size_t got=fread(buffer+pending,1,102400,stream);
        if(ferror(stream)){dbg("Read error");fclose(stream);return 0;}
        size_t total=pending+got;
        if(total>20){size_t payload=total-20;crc=hndcrc32(buffer,(unsigned int)payload,crc);memmove(buffer,buffer+payload,20);pending=20;}
        else pending=total;
        if(feof(stream))break;
        if(!got){dbg("Read error");fclose(stream);return 0;}
    }
    fclose(stream);
    return pending==20?crc:0;
}
