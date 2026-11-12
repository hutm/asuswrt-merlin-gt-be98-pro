/* SPDX-License-Identifier: GPL-2.0-only */
#include <string.h>
extern char *nvram_get(const char *);
char *ATE_FACTORY_MODE_STR(void)
{
    static char key[8];
    memcpy(key,"ATEMODE",sizeof(key));
    return key;
}
char *ATE_UPGRADE_MODE_STR(void)
{
    return strdup("ateUpgrade_flag");
}
int ate_brcm_factory_mode(void)
{
    char *value=nvram_get(ATE_FACTORY_MODE_STR());
    return value && !strcmp(value,"1");
}
