/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef OPEN_SPWENC_INPUTS_H
#define OPEN_SPWENC_INPUTS_H
struct nv_pw_inputs { char salt[33], product[64], mac[18]; };
struct nv_pw_input_ops {
 const char *(*salt)(void);
 const char *(*product)(void);
 const char *(*mac)(void);
 void (*delay)(unsigned microseconds);
};
/* Copies borrowed strings before subsequent reads. Output unchanged on error.
 * A denied salt pointer ((char *)-1) fails with EACCES. Callbacks must return
 * valid strings or NULL, except that documented salt sentinel. */
int nv_pw_inputs_load(const struct nv_pw_input_ops *, int with_salt,
                      struct nv_pw_inputs *);
#endif
