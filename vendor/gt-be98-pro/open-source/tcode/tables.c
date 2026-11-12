/* SPDX-License-Identifier: GPL-2.0-only */
#include <stdint.h>
#include "model.h"
struct defaults_entry {int model; char *odmpid, *territory, *name, *value;};
struct language_code {int model; char *territory, *location, *language;};
struct language_list {int model; char *odmpid, *territory, *languages; int fallback;};
struct location_entry {int model; char *territory, *format; int mode; char *country0, *revision0, *country1, *revision1, *country2, *revision2, *country3, *revision3;};
struct support_entry {int model; char *territory, *features;};
struct defaults_entry tcode_init_nvram_list[] = {
    {-1, "", "EE", "wl1_frameburst", "off"},
    {-1, "", "EU", "wl1_frameburst", "off"},
    {-1, "", "IL", "wl1_frameburst", "off"},
    {-1, "", "RU", "wl1_frameburst", "off"},
    {-1, "", "UA", "wl1_frameburst", "off"},
    {-1, "", "UK", "wl1_frameburst", "off"},
    {-1, "", "WE", "wl1_frameburst", "off"},
    {-1, "", "EE", "wl2_frameburst", "off"},
    {-1, "", "EU", "wl2_frameburst", "off"},
    {-1, "", "IL", "wl2_frameburst", "off"},
    {-1, "", "RU", "wl2_frameburst", "off"},
    {-1, "", "UA", "wl2_frameburst", "off"},
    {-1, "", "UK", "wl2_frameburst", "off"},
    {-1, "", "WE", "wl2_frameburst", "off"},
    {-1, "", "EU", "wl0_country_abbrev_override", "DE"},
    {-1, "", "EU", "wl1_country_abbrev_override", "DE"},
    {-1, "", "EU", "wl2_country_abbrev_override", "DE"},
    {-1, "", "EU", "wl3_country_abbrev_override", "DE"},
    {-1, "", "US", "wl0_bw_160", "1"},
    {-1, "", "KR", "wl0_bw_160", "1"},
    {-1, "", "EU", "wl0_bw_160", "1"},
    {-1, "", "CA", "wl0_bw_160", "1"},
    {-1, "", "AA", "wl0_bw_160", "1"},
    {-1, "", "AU", "wl0_bw_160", "1"},
    {-1, "", "US", "wl1_bw_160", "1"},
    {-1, "", "KR", "wl1_bw_160", "1"},
    {-1, "", "EU", "wl1_bw_160", "1"},
    {-1, "", "CA", "wl1_bw_160", "1"},
    {-1, "", "AA", "wl1_bw_160", "1"},
    {-1, "", "AU", "wl1_bw_160", "1"},
    {-1, "", "US", "wl2_bw_160", "1"},
    {-1, "", "KR", "wl2_bw_160", "1"},
    {-1, "", "EU", "wl2_bw_160", "1"},
    {-1, "", "CA", "wl2_bw_160", "1"},
    {-1, "", "AA", "wl2_bw_160", "1"},
    {-1, "", "AU", "wl2_bw_160", "1"},
    {-1, "", "AA", "wl1_bw_160", "1"},
    {-1, "", "ID", "wl1_bw_160", "0"},
    {-1, "", "AU", "wl1_bw_160", "1"},
    {-1, "", "CA", "wl1_bw_160", "1"},
    {-1, "", "CN", "wl1_bw_160", "1"},
    {-1, "", "JP", "wl1_bw_160", "1"},
    {-1, "", "KR", "wl1_bw_160", "1"},
    {-1, "", "S2", "wl1_bw_160", "1"},
    {-1, "", "SG", "wl1_bw_160", "1"},
    {-1, "", "TW", "wl1_bw_160", "1"},
    {-1, "", "US", "wl1_bw_160", "1"},
    {-1, "", "BZ", "wl1_bw_160", "1"},
    {-1, "", "CN", "wan_ppp_echo", "0"},
    {-1, "", "CN", "wan0_ppp_echo", "0"},
    {-1, "", "CN", "wan1_ppp_echo", "0"},
    {-1, "", "CN", "preferred_lang", "CN"},
    {-1, "", "CN", "ping_target", "www.baidu.com"},
    {-1, "", "CN", "dns_probe_host", "www.baidu.com"},
    {-1, "", "CN", "dns_probe_content", "*"},
    {-1, "", "AA", "dns_probe_content", "*"},
    {-1, "", "AA", "wandog_interval", "10"},
    {-1, "", "AA", "wandog_maxfail", "6"},
    {-1, "", "AA", "dns_probe_timeout", "8"},
    {-1, "", "AA", "http_enable", "2"},
    {-1, "", "JP", "preferred_lang", "JP"},
    {-1, "", "SG", "http_enable", "2"},
    {-1, "", "AA", "bcm_snooping", "0"},
    {-1, "", "SG", "bcm_snooping", "0"},
    {-1, "", "CN", "dns_ping_list", "<114DNS>114.114.114.114<114DNS>114.114.114.115<Ali DNS>223.5.5.5<Ali DNS>223.6.6.6<Baidu DNS>180.76.76.76"},
    {-1, "", "CT", "dns_ping_list", "<114DNS>114.114.114.114<114DNS>114.114.114.115<Ali DNS>223.5.5.5<Ali DNS>223.6.6.6<Baidu DNS>180.76.76.76"},
    {-1, "", "GD", "dns_ping_list", "<114DNS>114.114.114.114<114DNS>114.114.114.115<Ali DNS>223.5.5.5<Ali DNS>223.6.6.6<Baidu DNS>180.76.76.76"},
    {-1, "", "TC", "dns_ping_list", "<114DNS>114.114.114.114<114DNS>114.114.114.115<Ali DNS>223.5.5.5<Ali DNS>223.6.6.6<Baidu DNS>180.76.76.76"},
    {0, 0, 0, 0, 0},
};
struct defaults_entry location_init_nvram_list[] = {
    {-1, "", "RU", "acs_band3", "0"},
    {0, 0, 0, 0, 0},
};
struct language_code tcode_langcode_list[] = {
    {-1, "EE", "RU", "RU"},
    {-1, "EU", "RU", "RU"},
    {0, 0, 0, 0},
};
struct language_list tcode_lang_list[] = {
    {-1, 0, "CN", "CN EN", 0},
    {-1, 0, "JP", "JP EN", 0},
    {-1, 0, "GLOBAL", "BR CN CZ DE EN ES FR HU IT JP KR MS NL PL RU RO SL TH TR TW UK DA FI NO SV", 1},
    {0, 0, 0, 0, 0},
};
struct location_entry asusctrl_tcode_location_list[] = {
    {0, 0, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0},
};
struct location_entry tcode_location_list_HwIdB[] = {
    {0, 0, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0},
};
struct location_entry tcode_location_list_HwIdA[] = {
    {0, 0, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0},
};
struct location_entry tcode_location_list[] = {
    {MODEL_GTBE98_PRO, "US", "%d:%s", 1, "US", "130", "US", "130", "US", "131", "US", "132"},
    {MODEL_GTBE98_PRO, "CA", "%d:%s", 1, "CA", "210", "CA", "210", "CA", "211", "CA", "212"},
    {0, 0, 0, -1, 0, 0, 0, 0, 0, 0, 0, 0},
};
uint32_t tcode_del_rc_support_list_by_odmpid[4] = {0};
struct support_entry tcode_del_rc_support_list[] = {
    {-1, "BZ", "loclist"},
    {-1, "CA", "loclist"},
    {-1, "CZ", "loclist"},
    {-1, "DE", "loclist"},
    {-1, "EE", "loclist"},
    {-1, "EU", "loclist"},
    {-1, "JP", "loclist"},
    {-1, "ME", "loclist"},
    {-1, "NE", "loclist"},
    {-1, "RU", "loclist"},
    {-1, "SA", "loclist"},
    {-1, "SE", "loclist"},
    {-1, "TR", "loclist"},
    {-1, "TW", "loclist"},
    {-1, "UA", "loclist"},
    {-1, "UK", "loclist"},
    {-1, "US", "loclist"},
    {-1, "WE", "loclist"},
    {0, 0, 0},
};
struct support_entry tcode_rc_support_list[] = {
    {-1, "AA", "loclist"},
    {-1, "AP", "loclist"},
    {-1, "AQ", "loclist"},
    {-1, "CN", "loclist"},
    {-1, "AU", "loclist"},
    {-1, "HK", "loclist"},
    {-1, "IN", "loclist"},
    {-1, "KR", "defpsk loclist"},
    {-1, "MY", "loclist"},
    {-1, "S2", "loclist"},
    {-1, "SG", "loclist"},
    {-1, "RU", ""},
    {-1, "UA", ""},
    {-1, "EU", ""},
    {0, 0, 0},
};
struct defaults_entry ate_nvram_list[] = {
    {0, 0, 0, 0, 0},
};
struct defaults_entry tcode_nvram_list[] = {
    {0, 0, 0, 0, 0},
};
