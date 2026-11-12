#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
extern char *nvram_get(const char *);
extern char *nvram_default_get(const char *);
extern int nvram_get_int(const char *);
extern int d_exists(const char *);
extern int f_read_string(const char *, char *, int);
extern uint32_t rtk_get_phy_status(int);
extern int cprintf(const char *, ...);
uint32_t hnd_get_phy_status(char *ifname)
{
    char path[100], value[32] = {0};
    if (!ifname) return 0;
    snprintf(path, sizeof(path), "/sys/devices/virtual/net/%s/lower_eth1", ifname);
    if (ifname) {
        int mode = nvram_get("sw_mode") ? nvram_get_int("sw_mode") :
                   atoi(nvram_default_get("sw_mode"));
        if (mode == 1 && d_exists(path)) {
            if (!strcmp(ifname, "vlan2"))
                return rtk_get_phy_status(nvram_get_int("wans_lanport"));
            if (!strcmp(ifname, "vlan4094")) return rtk_get_phy_status(1);
        }
    }
    snprintf(path, sizeof(path), "/sys/devices/virtual/net/%s/lower_eth0", ifname);
    if (!strncmp(ifname, "br101", 5) || d_exists(path))
        strcpy(path, "/sys/class/net/eth0/operstate");
    else
        snprintf(path, sizeof(path), "/sys/class/net/%s/operstate", ifname);
    f_read_string(path, value, sizeof(value));
    if (!strncmp(value, "up", 2)) return 1;
    if (!strncmp(value, "down", 4)) return 0;
    cprintf("PID(%d) %s: Temporarily Router cannot get the PHY(%s) status...\n",
            (int)getpid(), __func__, ifname);
    syslog(LOG_NOTICE, "%s: Temporarily Router cannot get the PHY(%s) status...",
           __func__, ifname);
    return 1;
}
