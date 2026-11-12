#include <stddef.h>
#include <net/if.h>
#include <bcm/bcmswapitypes.h>
_Static_assert(offsetof(struct ethswctl_data,op)==0,"op");
_Static_assert(offsetof(struct ethswctl_data,unit)==8,"unit");
_Static_assert(offsetof(struct ethswctl_data,type)==28,"type");
_Static_assert(offsetof(struct ethswctl_data,data)==138,"data");
_Static_assert(offsetof(struct ethswctl_data,offset)==216,"offset");
_Static_assert(offsetof(struct ethswctl_data,length)==220,"length");
_Static_assert(sizeof(((struct ethswctl_data*)0)->data)==8,"data capacity");
int switch_struct_size(void){return sizeof(struct ethswctl_data);}
