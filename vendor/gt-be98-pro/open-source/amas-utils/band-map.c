#include <stdio.h>
#include <string.h>
extern int get_wl_count(void);
extern int nvram_get_int(const char *);
extern void get_control_channel(int, int *, int *, int *);
static int lookup(int value, const char *match, const char *result)
{
    for (int i = 0; i < get_wl_count(); ++i) {
        char full[64], prefix[13], key[64];
        snprintf(full, sizeof(full), "amas_wlc%d_", i);
        size_t n = strlen(full);
        if (n >= sizeof(prefix)) n = sizeof(prefix) - 1;
        memcpy(prefix, full, n); prefix[n] = 0;
        snprintf(key, sizeof(key), "%s%s", prefix, match);
        if (nvram_get_int(key) == value) {
            snprintf(key, sizeof(key), "%s%s", prefix, result);
            return nvram_get_int(key);
        }
    }
    return -1;
}
int get_unit_by_wlc_bandindex(int index) { return lookup(index, "index", "unit"); }
int get_wlc_bandindex_by_unit(int unit) { return lookup(unit, "unit", "index"); }
int get_type_by_channel(int unit, int band)
{
    int channel = 0, bandwidth = 0, sideband = 0;
    get_control_channel(unit, &channel, &bandwidth, &sideband);
    switch (band) {
    case 2: return channel >= 13 ? 2 : 1;
    case 1: return channel >= 100 ? 2 : 1;
    case 4: return channel >= 129 ? 2 : 1;
    default: return 0;
    }
}
