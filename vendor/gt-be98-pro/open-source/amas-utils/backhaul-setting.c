extern int nvram_get_int(const char *), get_wl_count(void), nvram_set(const char *, const char *), nvram_commit(void);
void trans_to_amas_bhmode(void)
{
    int ethernet = nvram_get_int("amas_ethernet");
    int count = get_wl_count();
    const char *value = ethernet == 1 ? (count == 2 ? "320000" : "540000") : (count == 2 ? "303100" : "503100");
    nvram_set("amas_bhmode", value);
    nvram_commit();
}
