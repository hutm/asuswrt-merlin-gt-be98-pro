#include <string.h>
extern int f_read_string(const char *,char *,int);
int hnd_boardid_cmp(const char *boardid)
{
    if(!boardid)return -1;
    char current[32]={0};
    if(!f_read_string("/proc/nvram/boardid",current,sizeof(current)))return -1;
    if(strlen(boardid)!=strlen(current))return -1;
    return strncmp(boardid,current,sizeof(current));
}
