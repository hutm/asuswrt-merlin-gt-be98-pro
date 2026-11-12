/* SPDX-License-Identifier: GPL-2.0-only */
#include <stddef.h>
struct hw_entry {char *field[9];};
struct sw_entry {char *name,*id,*key;unsigned flags;};
struct mode_entry {char *product;int mode;};
struct hw_entry s_hw_auth_tuple[]={
 {{"GT-BE98_Pro","4109","4108","","255","GT-BE98 PRO","GT-BE98 PRO","GT-BE98 PRO",""}},
 {{NULL}}
};
struct sw_entry s_sw_auth_tuple[]={
 {"httpd","45646223","fk309g1sedk0353445g",0},
 {"mastiff","14354641","mg8rla5fj94kq0kcm2z",0},
 {"aaews","89347542","jidf0924ij4pdfg54as",0},
 {"wlc_nt","25124577","afa125g46h4yefse03t",0},
 {"AMASH","33716237","g2hkhuig238789ajkhc",0},
 {NULL,NULL,NULL,0}
};
char *s_tunnel_whitelist_tuple[]={"GT-BE98_Pro",NULL};
struct mode_entry s_amas_whitelist_tuple[]={{"GT-BE98_Pro",3},{NULL,0}};
