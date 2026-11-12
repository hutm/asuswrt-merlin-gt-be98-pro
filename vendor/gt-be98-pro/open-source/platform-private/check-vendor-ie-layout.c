#include <stddef.h>
#include <typedefs.h>
#include <wlioctl.h>
_Static_assert(offsetof(vndr_ie_info_t,pktflag)==0,"flag");
_Static_assert(offsetof(vndr_ie_info_t,vndr_ie_data)==10,"IE");
_Static_assert(offsetof(vndr_ie_info_t,vndr_ie_data.oui)==12,"OUI");
_Static_assert(offsetof(vndr_ie_info_t,vndr_ie_data.data)==15,"data");
_Static_assert(offsetof(vndr_ie_buf_t,vndr_ie_list)==4,"list");
_Static_assert(offsetof(vndr_ie_setbuf_t,vndr_ie_buffer)==4,"set list");
int main(void){return 0;}
