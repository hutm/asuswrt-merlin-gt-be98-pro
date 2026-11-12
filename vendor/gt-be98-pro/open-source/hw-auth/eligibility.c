/* SPDX-License-Identifier: GPL-2.0-only */
#include <stdlib.h>
#include <string.h>
struct mode_entry {char *product;int mode;};
int GPIO_LOWACTIVE_COMPARE(char *left,char *right)
{
 return (((unsigned)atoi(left) ^ (unsigned)atoi(right)) & 4095u)==0;
}
static int internal_mode(const char *product,const struct mode_entry *table)
{
 for(;table->product;table++){
  if(!strncmp(product,"RT-AC68",7)&&strncmp(product,"RT-AC68U_V4",11))return table->mode;
  if(!strcmp(table->product,product))return table->mode;
 }
 return 0;
}

extern char *get_productid(void);
extern struct mode_entry s_amas_whitelist_tuple[];
int getAmasSupportMode(void){return internal_mode(get_productid(),s_amas_whitelist_tuple);}
