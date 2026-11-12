#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern int f_read_string(const char *path, char *buffer, int max);

/* This component is specific to the experimental GT-BE98 Pro profile. */
int is_rtl8372_boardid(void) { return 1; }
uint32_t hnd_get_phy_speed(char *ifname)
{
    char path[100], value[32] = {0};
    snprintf(path, sizeof(path), "/sys/class/net/%s/speed", ifname);
    f_read_string(path, value, sizeof(value));
    return (uint32_t)strtoul(value, NULL, 10);
}
uint32_t hnd_get_phy_duplex(char *ifname)
{
    char path[100], value[32] = {0};
    snprintf(path, sizeof(path), "/sys/class/net/%s/duplex", ifname);
    f_read_string(path, value, sizeof(value));
    return strncmp(value, "full", 4) == 0;
}
uint64_t hnd_get_phy_mib(char *ifname, char *type)
{
    char path[100], value[32] = {0};
    if (!ifname || !type) return 0;
    snprintf(path, sizeof(path), "/sys/class/net/%s/statistics/%s", ifname, type);
    f_read_string(path, value, sizeof(value));
    return strtoull(value, NULL, 10);
}
