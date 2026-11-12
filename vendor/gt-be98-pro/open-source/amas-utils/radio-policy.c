#include <stdio.h>
#include <string.h>
extern int num_of_wl_if(void), nvram_get_int(const char *), isMloConnectionMode(void), is_mlo_map(char *);
extern char *nvram_pf_get(const char *, const char *);
extern int get_wifi_2G_maxpower(void), get_wifi_6GH_maxpower(void);
extern double get_wifi_5G_maxpower(void), get_wifi_5GH_maxpower(void), get_wifi_6G_maxpower(void);
static void copy_prefix(char *out, size_t size, const char *fmt, int value)
{
    char full[64]; snprintf(full, sizeof(full), fmt, value);
    size_t n = strlen(full); if (n >= size) n = size - 1;
    memcpy(out, full, n); out[n] = 0;
}
int get_mlo_fh_band(void)
{
    int two = 0, five = 0, six = 0, count = num_of_wl_if();
    for (int i = 0; i < count; ++i) {
        char prefix[9], key[32];
        copy_prefix(prefix, sizeof(prefix), "wl%d_", i);
        snprintf(key, sizeof(key), "%snband", prefix);
        switch (nvram_get_int(key)) { case 2: ++two; break; case 1: ++five; break; case 4: ++six; break; }
    }
    int mask = (two == 1) | (five == 1 ? 2 : five == 2 ? 12 : 0) | (six == 1 ? 16 : six == 2 ? 96 : 0);
    switch (mask) { case 29: return 25; case 99: return 35; case 19: return 19; case 13: return 13; case 3: return 3; default: return 0; }
}
int is_mlo_map_by_bandindex(int index)
{
    if (!isMloConnectionMode()) return 0;
    char prefix[13], name[8];
    copy_prefix(prefix, sizeof(prefix), "amas_wlc%d", index);
    char *value = nvram_pf_get(prefix, "_ifname");
    snprintf(name, sizeof(name), "%s", value ? value : "(null)");
    return is_mlo_map(name) != 0;
}
double get_wifi_tx_maxpower(int band)
{
    switch (band) {
    case 2: return get_wifi_2G_maxpower();
    case 5: return get_wifi_5G_maxpower();
    case 52: return get_wifi_5GH_maxpower();
    case 6: return get_wifi_6G_maxpower();
    case 62: return get_wifi_6GH_maxpower();
    default: return 0.0;
    }
}
