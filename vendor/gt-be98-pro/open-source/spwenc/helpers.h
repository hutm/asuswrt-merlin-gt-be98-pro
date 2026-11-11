/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef OPEN_SPWENC_HELPERS_H
#define OPEN_SPWENC_HELPERS_H
char *str_to_md5(const char *input,int length,char *output);
int is_same_pass(const char *name,const char *user,const char *password);
void get_encrypt_list_value(const char *name,const char *user,char *output,int capacity);
#endif
