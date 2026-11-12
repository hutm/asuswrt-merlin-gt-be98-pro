#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <syslog.h>
#include <unistd.h>
extern int f_read_string(const char *, char *, int);
extern uint32_t rtk_get_phy_status(int);
extern int cprintf(const char *, ...);
int ethctl_get_link_status(char *ifname)
{
    char path[100], value[32] = {0};
    if (!strcmp(ifname, "vlan4094")) return rtk_get_phy_status(1);
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
