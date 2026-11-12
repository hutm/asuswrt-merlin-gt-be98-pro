#ifndef OPEN_WGN_VLAN_H
#define OPEN_WGN_VLAN_H
#include <stddef.h>
#include <stdint.h>
typedef struct {
 unsigned enable,vid;char prio;
 char wanportset[9],lanportset[9],wl2gset[33],wl5gset[33],subnet_name[33];
 unsigned internet,public_vlan,type;
} wgn_vlan_rule;
_Static_assert(sizeof(wgn_vlan_rule)==140,"VLAN ABI");
_Static_assert(offsetof(wgn_vlan_rule,subnet_name)==93,"VLAN subnet ABI");
_Static_assert(offsetof(wgn_vlan_rule,internet)==128,"VLAN internet ABI");
#endif
