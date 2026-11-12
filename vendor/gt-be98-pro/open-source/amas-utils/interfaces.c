#include <stdio.h>
#include <string.h>
#include <stddef.h>
extern char *nvram_get(const char *);
extern int nvram_get_int(const char *);
extern char *get_compatible_network(int, char *, int);
extern char *get_fh_if_prefix(char *, size_t);
int wl_unit(char *ifname)
{
    if (!ifname) return -1;
    for (int unit = 0; unit < 5; ++unit) {
        char key[16];
        snprintf(key, sizeof(key), "wl%d_ifname", unit);
        const char *value = nvram_get(key);
        if (value && *value && !strncmp(value, ifname, strlen(ifname))) return unit;
    }
    return -1;
}
int is_mlo_dwb_mssid(char *ifname)
{
    int band = nvram_get_int("dwb_band");
    int subunit = nvram_get_int("mlo_dwb_mssid_subunit");
    char full[64], key[17];
    snprintf(full, sizeof(full), "wl%d.%d_ifname", band, subunit);
    size_t n = strlen(full);
    if (n >= sizeof(key)) n = sizeof(key) - 1;
    memcpy(key, full, n); key[n] = 0;
    const char *value = nvram_get(key);
    return ifname && !strcmp(value ? value : "", ifname);
}
static int contains(const char *list, const char *name)
{
    if (!name) return 0;
    while (*list) {
        list += strspn(list, " ");
        size_t n = strcspn(list, " ");
        if (!n) break;
        if (strlen(name) == n && !memcmp(list, name, n)) return 1;
        list += n;
    }
    return 0;
}
int is_compatible_network(char *name)
{
    char list[32] = {0};
    if (!get_compatible_network(-1, list, sizeof(list))) return 0;
    list[31] = 0;
    return contains(list, name);
}
int is_mainFH_network(char *name)
{
    char list[32] = {0};
    if (!get_fh_if_prefix(list, sizeof(list))) return 0;
    list[31] = 0;
    return contains(list, name);
}
