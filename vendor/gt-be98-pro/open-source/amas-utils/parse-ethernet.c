#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
extern void trans_to_bhmode(int *, int *, int *, int *, int *);
int cal_space(char *text)
{
    if (!text) return 0;
    printf("s1 = %s\n", text);
    size_t n = strlen(text);
    if (!n || isspace((unsigned char)text[0]) || isspace((unsigned char)text[n-1])) {
        puts("config format is incorrect.");
        return 0;
    }
    int count = 1;
    for (size_t i = 0; i < n; ++i) if (isspace((unsigned char)text[i])) ++count;
    printf("parameter count = %d\n", count);
    return count;
}
int enable_ETH_U(int unit)
{
    int eth = 0, wifi = 0, plc = 0, moca = 0, other = 0;
    trans_to_bhmode(&eth, &wifi, &plc, &moca, &other);
    uint32_t adjusted = (uint32_t)unit + (unit < 0 ? 3u : 0u);
    unsigned shift = ((adjusted & ~3u) + 4u + (uint32_t)unit) & 255u;
    return shift < 31 && (((uint32_t)eth & (UINT32_C(1) << shift)) != 0);
}
