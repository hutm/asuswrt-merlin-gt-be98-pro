/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef OPEN_SPWENC_POLICY_H
#define OPEN_SPWENC_POLICY_H
struct ASUS_PP_table {char *name,*version;int id,force_sign;};
extern struct ASUS_PP_table ASUS_PP_t[];
int get_ASUS_privacy_policy_state(int id);
int get_ASUS_privacy_policy(void);
int get_ASUS_privacy_policy_ver(int id);
#endif
