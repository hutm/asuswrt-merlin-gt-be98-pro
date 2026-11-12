#include <stdio.h>
#include <string.h>
extern char *nvram_get(const char *);
extern int nvram_set(const char *, const char *), num_of_5g_if(void);
extern int _add_to_list(const char *, char *, int, char);
extern int cprintf(const char *, ...);
int acs_set_chwt(int channel)
{
    int band = channel <= 51 ? 1 : channel <= 99 ? 2 : channel <= 148 ? 3 : channel < 169 ? 4 : 5;
    const char *value = nvram_get("disable_amas_chwt");
    if (value && !strcmp(value, "1")) return -1;
    if (band != 2 && band != 3) {
        cprintf("%s: invalid band:%d\n", "acs_set_chwt", band);
        return -1;
    }
    int count = num_of_5g_if();
    if (!(band == 3 && count == 2) && num_of_5g_if() != 1) {
        cprintf("%s: invalid unit:%d\n", "acs_set_chwt", -1);
        return -1;
    }
    char list[512], number[8], weight[8];
    value = nvram_get("wl0_acs_channel_weights");
    snprintf(list, sizeof(list), "%s", value ? value : "");
    char full[32];
    snprintf(full, sizeof(full), "%d", channel);
    size_t n = strlen(full); if (n >= sizeof(number)) n = sizeof(number)-1;
    memcpy(number, full, n); number[n] = 0;
    value = nvram_get("acs_chwt");
    snprintf(weight, sizeof(weight), "%s", value ? value : "85");
    _add_to_list(number, list, sizeof(list), ',');
    _add_to_list(weight, list, sizeof(list), ',');
    cprintf("%s, set %s chwt as %s\n", "acs_set_chwt", "wl0_", list);
    nvram_set("wl0_acs_channel_weights", list);
    return 0;
}
