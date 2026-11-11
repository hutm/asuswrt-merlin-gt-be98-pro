/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef OPEN_SPWENC_NAMES_H
#define OPEN_SPWENC_NAMES_H
#include <stddef.h>
int webapi_get_b(int id,char *buffer,size_t capacity);
/* Append promoted character values, terminated by a zero low byte. */
void c(char *buffer,size_t capacity,...);
#endif
