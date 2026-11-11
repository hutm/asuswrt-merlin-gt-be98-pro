/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef OPEN_HTTPD_PWENC_H
#define OPEN_HTTPD_PWENC_H
char *base64(const void *input, int length);
int aes_encrypt(const char *input, const unsigned char *key, unsigned char *output);
char *pwenc(char *input, char *output, int capacity);
int gen_ddns_hostname(char *output, int capacity);
#endif
