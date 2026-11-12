#ifndef OPEN_WGN_SUBNET_H
#define OPEN_WGN_SUBNET_H
#include <stddef.h>
#include <stdint.h>
typedef struct {
 char ipaddr[17],ipmask[17];unsigned dhcp_enable;char dhcp_start[17],dhcp_end[17];unsigned dhcp_lease;
 char domain_name[133],dns[133],wins[133];unsigned ema;char macipbinding[1025];unsigned type;
} wgn_subnet_rule;
_Static_assert(sizeof(wgn_subnet_rule)==1516,"subnet ABI");
_Static_assert(offsetof(wgn_subnet_rule,type)==1512,"subnet type ABI");
#endif
