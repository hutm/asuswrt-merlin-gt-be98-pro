#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
extern int d_exists(const char *), nvram_set_int(const char *, int);
extern char *nvram_get(const char *), *wl_nvname(const char *, int, int);
extern int isMloConnectionMode(void), is_mlo_if(char *), is_mlo_map(char *);
extern char *nvram_pf_get(const char *, const char *);
void create_amas_sys_folder(void)
{
    if (!d_exists("/jffs/.sys")) (void)mkdir("/jffs/.sys", 0666);
}
void trigger_opt(void)
{
    char list[32]; const char *value = nvram_get("wl_ifnames");
    snprintf(list, sizeof(list), "%s", value ? value : "");
    const char *p = list;
    int unit = 0;
    while (*p) {
        p += strspn(p, " ");
        size_t n = strcspn(p, " ");
        if (!n) break;
        if (unit <= 3 && nvram_get(wl_nvname("nband", unit, 0))) {
            char key[64]; snprintf(key, sizeof(key), "amas_wlc%d_optmz", unit);
            nvram_set_int(key, 2);
        }
        ++unit; p += n;
    }
}
int check_wifi_use(int index)
{
    if (!isMloConnectionMode()) return 0;
    char full[64], prefix[13], name[8];
    snprintf(full, sizeof(full), "amas_wlc%d", index);
    size_t n = strlen(full); if (n >= sizeof(prefix)) n = sizeof(prefix)-1;
    memcpy(prefix, full, n); prefix[n] = 0;
    char *value = nvram_pf_get(prefix, "_ifname");
    snprintf(name, sizeof(name), "%s", value ? value : "(null)");
    if (is_mlo_if(name) && is_mlo_map(name)) return 1;
    return is_mlo_if(name) == 0;
}
