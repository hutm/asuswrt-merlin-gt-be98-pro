#include <stdio.h>
#include <string.h>
extern int f_read_string(const char *,char *,int);
int check_pkgtb_boardid(char *image)
{
    if(!image)return -1;
    char marker[255]="fdt_";
    int read=f_read_string("/proc/nvram/boardid",marker+4,251);
    size_t length=strlen(marker);
    if(read>0&&length>4){
        for(size_t offset=0;offset+length<=16384;offset++){
            if(!memcmp(image+offset,marker,length)){
                fprintf(stderr,"[%s][%d] [%s] in firmware\n","check_pkgtb_boardid",3203,marker);return 1;
            }
        }
    }
    fprintf(stderr,"[%s][%d] [%s] doesn't exist in firmware\n","check_pkgtb_boardid",3208,marker);return -1;
}
