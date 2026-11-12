#include <stdint.h>
#include <stdio.h>
#include <string.h>
extern void dbg(const char *,...);
/* The model object uses a numeric second argument, unlike the legacy header. */
int check_trx(char *filename,unsigned int expected)
{
    if(!filename)return 0;
    FILE *stream=fopen(filename,"r");
    if(!stream){dbg("Open trx fail!!!\n");return 0;}
    unsigned char header[28],first,second;
    if(fread(header,1,sizeof(header),stream)!=sizeof(header)){dbg("Read header error!!!\n");fclose(stream);return 0;}
    uint32_t length;memcpy(&length,header+4,4);
    uint32_t first_offset=length>9444?9444:16;
    uint32_t second_offset=length>0x8fdc30?0x8fdc30:length>9444?((length-16)/4-2357+4)*4:16;
    if(fseek(stream,(long)first_offset,SEEK_SET)||fread(&first,1,1,stream)!=1||
       fseek(stream,(long)second_offset,SEEK_SET)||fread(&second,1,1,stream)!=1){fclose(stream);return 0;}
    unsigned char value=second?(unsigned char)(first-1-second):(unsigned char)(first%3-3);
    fclose(stream);return expected==value;
}
