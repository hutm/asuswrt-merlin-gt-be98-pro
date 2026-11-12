#include <string.h>
extern char *nvram_get(const char *);
extern int hnd_get_phy_status(char *);
static const char *safe(const char *key){const char*v=nvram_get(key);return v?v:"";}
static int collect(const char *key)
{
    const char *first=safe(key);size_t skip=strspn(safe(key)," ");
    size_t len=strlen(first);if(skip>len)skip=len;
    const char *next=first+skip;int status=0;
    for(;;){
        char name[100];strncpy(name,next,99);name[99]=0;name[strcspn(name," ")]=0;
        const char *following=strchr(next,' ');
        if(!name[0])break;
        status|=hnd_get_phy_status(name);
        next=following?following+strspn(following," "):"";
    }
    return status;
}
int get_port_status_hnd(void)
{
    int status=collect("wan_ifnames");return status|collect("lan_ifnames");
}
