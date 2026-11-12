#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <syslog.h>
extern int nvram_get_int(const char *), nvram_set_int(const char *, int), wl_unit(char *);
extern uint8_t wf_chspec_primary20_chan(uint16_t);
void acsd_amasinfo(char *ifname, uint16_t spec)
{
    int repeater = nvram_get_int("re_mode");
    char prefix[20], key[40];
    if (repeater) {
        if (!ifname || strlen(ifname) < 3) return;
        snprintf(prefix, sizeof(prefix), "wl%c_", (unsigned char)ifname[2]);
    } else {
        int unit = wl_unit(ifname);
        if (unit < 0) { syslog(LOG_NOTICE, "amasinfo invalid wlif:%s\n", ifname ? ifname : "(null)"); return; }
        snprintf(prefix, sizeof(prefix), "wl%d_", unit);
    }
    int channel = wf_chspec_primary20_chan(spec), bandwidth = 0, sideband = 0;
    switch (spec & 0x3800) {
    case 0x1000: bandwidth = 20; break;
    case 0x1800: bandwidth = 40; sideband = (spec & 0x3f00) == 0x1900; break;
    case 0x2000: bandwidth = 80; break;
    case 0x2800: bandwidth = 160; break;
    case 0x3000: bandwidth = 320; break;
    }
    if (nvram_get_int("acsd_debug") & 1)
        syslog(LOG_NOTICE, "(re:%d)wl_prefix=%s, amas bw=%d, nsb=%d\n", repeater, prefix, bandwidth, sideband);
    snprintf(key, sizeof(key), "%ssel_channel", prefix); nvram_set_int(key, channel);
    snprintf(key, sizeof(key), "%ssel_bw", prefix); nvram_set_int(key, bandwidth);
    snprintf(key, sizeof(key), "%ssel_nctrlsb", prefix); nvram_set_int(key, sideband);
}
