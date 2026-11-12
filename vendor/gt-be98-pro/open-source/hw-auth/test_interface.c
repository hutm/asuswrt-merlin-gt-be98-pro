/* SPDX-License-Identifier: GPL-2.0-only */
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "model.h"
extern char *hw_component_clm_2g(int);
static char *interface;static unsigned commands,reads;
char *wl_nvname(const char *name,int unit,int sub){(void)name;(void)unit;(void)sub;return "wl_ifname";}
char *nvram_get(const char *key){assert(!strcmp(key,"wl_ifname"));return interface;}
int system(const char *command){(void)command;commands++;return -1;}
int f_read_string(const char *path,char *out,int cap){(void)path;reads++;snprintf(out,cap,"version");return 7;}
int main(void){
 char *bad[]={NULL,"","eth0;id","eth0\nwhoami","$(id)","`id`","eth0 foo","eth0>file","eth0&x","eth0|x","eth0'","eth0\"","0123456789012345"};
 for(unsigned i=0;i<sizeof(bad)/sizeof(bad[0]);i++){interface=bad[i];commands=reads=0;assert(!*hw_component_clm_2g(MODEL_RTBE86U));assert(!commands&&!reads);}
 interface="012345678901234";commands=reads=0;assert(!strcmp(hw_component_clm_2g(MODEL_RTBE86U),"version"));assert(commands==1&&reads==1);
 puts("14 interface validation cases passed");return 0;
}
