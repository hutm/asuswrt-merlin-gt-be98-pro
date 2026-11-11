/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef OPEN_SPWENC_COMMANDS_H
#define OPEN_SPWENC_COMMANDS_H
int safe_do_system(const char *,...);
int safe_fork_do_system(const char *,...);
int safe_do_system_to_file(const char *,int,const char *,...);
#endif
