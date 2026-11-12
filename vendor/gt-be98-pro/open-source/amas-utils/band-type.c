#include <stdio.h>
extern int num_of_wl_if(void), nvram_get_int(const char *), nvram_set_int(const char *, int);
extern char *nvram_get(const char *), *wl_nvname(const char *, int, int);
extern int get_bandnum_by_nband(int), get_type_by_channel(int, int);
void check_wlx_nband_type(void)
{
    int count = num_of_wl_if();
    for (int unit = 0; unit < count; ++unit) {
        if (unit > 3 || !nvram_get(wl_nvname("nband", unit, 0))) { ++unit; continue; }
        char key[64]; snprintf(key, sizeof(key), "wl%d_nband", unit);
        int band = nvram_get_int(key), value;
        if (band == 2) value = 0;
        else if (band == 1 || band == 4) {
            if (get_bandnum_by_nband(band) == 1) value = band == 1 ? 1 : 4;
            else {
                int type = get_type_by_channel(unit, band);
                if (type != 1 && type != 2) continue;
                value = (band == 1 ? 1 : 4) + type;
            }
        } else continue;
        snprintf(key, sizeof(key), "wl%d_nband_type", unit);
        nvram_set_int(key, value);
    }
}
