/* SPDX-License-Identifier: GPL-2.0-only */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <openssl/sha.h>
#include "auth_common.h"
char *get_auth_code(char *initial, char *out, int capacity)
{
    char copy[513] = {0}, message[513];
    int rounds;
    if (!out || capacity < 65 || capacity > 513) {
        errno = EINVAL;
        return NULL;
    }
    if (!initial || strlen(initial) > 512) {
        printf("The initial_value is NULL or the length of initial_value is greater than %d.\n",512);
        srand(time(NULL));
        rounds = rand() % 200 + 20;
        snprintf(copy,512,"%d",rounds);
        initial = copy;
    } else {
        strncpy(copy,initial,512);
        /* Preserve the legacy first-token parsing and existing errno behavior. */
        char *token = strtok(copy,"|");
        long long value = token ? strtoll(token,NULL,10) : 0;
        if (!value || errno == ERANGE) {
            srand(time(NULL));
            rounds = rand() % 200 + 20;
        } else {
            rounds = value % 200 + 20;
        }
    }
    memset(out,0,capacity);
    /* Negative parsed values can select fewer than one round: still hash once. */
    for (int i=0;;i++) {
        unsigned char digest[SHA256_DIGEST_LENGTH];
        snprintf(message,capacity,"%s|%s|%d",out,initial,i);
        if (!SHA256((unsigned char *)message,strlen(message),digest)) {
            memset(out,0,capacity);
            errno = EIO;
            return NULL;
        }
        for (unsigned j=0;j<sizeof(digest);j++)
            sprintf(out+2*j,"%2.2x",digest[j]);
        out[64]=0;
        if(i+1>=rounds)break;
    }
    return out;
}
