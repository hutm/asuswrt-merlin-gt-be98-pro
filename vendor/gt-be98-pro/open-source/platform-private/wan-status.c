#include <stdint.h>
#include <stdlib.h>
extern char *nvram_get(const char *);
extern char *nvram_default_get(const char *);
extern int nvram_get_int(const char *);
extern char *get_wanx_ifname(int);
extern uint32_t hnd_get_phy_status(char *);
uint32_t wanport_status_brcm(int unit)
{
    int mode = nvram_get("sw_mode") ? nvram_get_int("sw_mode") :
               atoi(nvram_default_get("sw_mode"));
    if (mode == 1) return hnd_get_phy_status(get_wanx_ifname(unit));
    return 1;
}
