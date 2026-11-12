#include <stdio.h>
#include <string.h>
extern int num_of_wl_if(void), nvram_get_int(const char *), nvram_set_int(const char *, int), nvram_unset(const char *);
static void copy_prefix(char *out, size_t size, const char *fmt, int value)
{
    char full[64]; snprintf(full, sizeof(full), fmt, value);
    size_t n = strlen(full); if (n >= size) n = size - 1;
    memcpy(out, full, n); out[n] = 0;
}
int gen_mlo_band(void)
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
void check_mlo_config(void)
{
    int band = gen_mlo_band();
    if (!nvram_get_int("mld_enable")) nvram_unset("mlo_config");
    else if (nvram_get_int("mlo_config") != band) nvram_set_int("mlo_config", band);
}
