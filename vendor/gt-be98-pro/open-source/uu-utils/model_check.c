/* SPDX-License-Identifier: GPL-2.0-only */
#include <stdio.h>
#include <string.h>
#include "model.h"
extern int get_model(void);
extern char *get_productid(void);
int uu_model_check(void)
{
    int model = get_model();
    const char *id = get_productid();
    char product[32];
    snprintf(product, sizeof(product), "%s", id ? id : "");
    if (strstr(product, "ExpertWiFi"))
        return 0;
    switch (model) {
    case MODEL_RTAC82U: return !strcmp(product, "RT-AC2200");
    case MODEL_RTAC68U: return strcmp(product, "RP-AC1900") != 0;
    case MODEL_RTAX58U: return strcmp(product, "RT-AX58U") != 0;
    case MODEL_RTAX82U_V2: return !strcmp(product, "RT-AX82U");
    case MODEL_TUFAX5400_V2: return !strcmp(product, "TUF-AX5400");
    case MODEL_RTAX5400: return !strcmp(product, "RT-AX5400");
    case MODEL_RTAX82_XD6S:
    case MODEL_XD6_V2: return !strcmp(product, "ZenWiFi_XD6");
    case MODEL_EBA63:
    case MODEL_BM68:
    case MODEL_BR63:
    case MODEL_EBG19:
    case MODEL_EBG15: return 0;
    default: return 1;
    }
}
