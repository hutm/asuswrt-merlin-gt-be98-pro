#include <stddef.h>
#include <bcmTag.h>
_Static_assert(sizeof(WFI_TAG)==20,"tag length");
_Static_assert(offsetof(WFI_TAG,wfiCrc)==0,"CRC");
_Static_assert(offsetof(WFI_TAG,wfiVersion)==4,"version");
_Static_assert(offsetof(WFI_TAG,wfiChipId)==8,"chip");
_Static_assert(offsetof(WFI_TAG,wfiFlashType)==12,"flash");
_Static_assert(offsetof(WFI_TAG,wfiFlags)==16,"flags");
int main(void){return 0;}
