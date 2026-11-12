extern int nvram_set_int(const char *, int);
void amas_stop_acsd_config_init(int model)
{
    switch (model) {
    case 170: case 175:
        nvram_set_int("amas_wl3_noacsd", 1);
        nvram_set_int("amas_wl0_noacsd", 1);
        nvram_set_int("amas_wl1_noacsd", 1);
        nvram_set_int("amas_wl2_noacsd", 1);
        break;
    case 101: case 138: case 155: case 180:
        nvram_set_int("amas_wl0_noacsd", 1);
        nvram_set_int("amas_wl1_noacsd", 1);
        nvram_set_int("amas_wl2_noacsd", 1);
        break;
    case 100: case 133: case 142: case 153: case 169:
        nvram_set_int("amas_wl0_noacsd", 1);
        nvram_set_int("amas_wl2_noacsd", 1);
        break;
    }
}
