/* SPDX-License-Identifier: GPL-2.0-only */
#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
struct hw_entry {char *field[9];};
struct sw_entry {char *name,*id,*key;unsigned flags;};
struct mode_entry {char *product;int mode;};
static unsigned checks;
static void same(char *a,char *b){assert((a==NULL)==(b==NULL));if(a)assert(!strcmp(a,b));checks++;}
int main(int argc,char **argv){
 assert(argc==3);
 void *a=dlopen(argv[1],RTLD_LAZY),*b=dlopen(argv[2],RTLD_LAZY);assert(a&&b);
 struct hw_entry *ha=dlsym(a,"s_hw_auth_tuple"),*hb=dlsym(b,"s_hw_auth_tuple");assert(ha&&hb);
 for(unsigned i=0;i<2;i++)for(unsigned j=0;j<9;j++)same(ha[i].field[j],hb[i].field[j]);
 struct sw_entry *sa=dlsym(a,"s_sw_auth_tuple"),*sb=dlsym(b,"s_sw_auth_tuple");assert(sa&&sb);
 for(unsigned i=0;i<6;i++){same(sa[i].name,sb[i].name);same(sa[i].id,sb[i].id);same(sa[i].key,sb[i].key);assert(sa[i].flags==sb[i].flags);checks++;}
 struct mode_entry *ma=dlsym(a,"s_amas_whitelist_tuple"),*mb=dlsym(b,"s_amas_whitelist_tuple");assert(ma&&mb);
 for(unsigned i=0;i<2;i++){same(ma[i].product,mb[i].product);assert(ma[i].mode==mb[i].mode);checks++;}
 char **ta=dlsym(a,"s_tunnel_whitelist_tuple"),**tb=dlsym(b,"s_tunnel_whitelist_tuple");assert(ta&&tb);for(unsigned i=0;i<2;i++)same(ta[i],tb[i]);
 printf("%u public table fields match\n",checks);return 0;
}
