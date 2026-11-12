#include <stdint.h>
#include <string.h>
struct uplink_entry { char name[32]; int value; };
struct uplink_entry port_define[] = {
    {"NONE", 0},
    {"WAN", 1},
    {"LAN", 2},
    {"", -1},
};
struct uplink_entry phy_type[] = {
    {"NONE", 0},
    {"ETH", 1},
    {"WIFI", 2},
    {"PLC", 3},
    {"MOCA", 4},
    {"", -1},
};
struct uplink_entry phy_moca_subtype[] = {
    {"NONE", 0},
    {"", -1},
};
struct uplink_entry phy_eth_subtype[] = {
    {"NONE", 0},
    {"10", 1},
    {"100", 2},
    {"1000", 3},
    {"1G", 3},
    {"2.5G", 4},
    {"5G", 5},
    {"10G", 6},
    {"10GSFP+", 7},
    {"", -1},
};
struct uplink_entry phy_wifi_subtype[] = {
    {"NONE", 0},
    {"2.4G", 1},
    {"5G", 2},
    {"6G", 3},
    {"", -1},
};
struct uplink_entry phy_plc_subtype[] = {
    {"NONE", 0},
    {"", -1},
};
static uint32_t lookup(const struct uplink_entry *table, const char *name)
{
    if (name) for (; table->value != -1; ++table) if (!strcmp(table->name, name)) return (uint32_t)table->value;
    return 0;
}
int gen_uplinkport_describe(char *port, char *type, char *subtype, int index)
{
    uint32_t value = lookup(port_define, port) << 16;
    value += lookup(phy_type, type) << 12;
    const struct uplink_entry *table = 0;
    if (type && subtype) {
        if (!strcmp(type, "ETH")) table = phy_eth_subtype;
        else if (!strcmp(type, "WIFI")) table = phy_wifi_subtype;
        else if (!strcmp(type, "PLC")) table = phy_plc_subtype;
        else if (!strcmp(type, "MOCA")) table = phy_moca_subtype;
    }
    if (table) value += lookup(table, subtype) << 8;
    value += (uint32_t)index;
    int result; memcpy(&result, &value, sizeof(result)); return result;
}
