/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef OPEN_SPWENC_AES_ABI_H
#define OPEN_SPWENC_AES_ABI_H
struct nv_aes_context {unsigned char round_key[240];};
void AES_init_ctx(struct nv_aes_context *,const unsigned char key[32]);
void AES_ECB_encrypt(const struct nv_aes_context *,unsigned char block[16]);
void AES_ECB_decrypt(const struct nv_aes_context *,unsigned char block[16]);
/* Caller supplies ceil(length/16)*16 bytes. A final short block is zero-filled
 * before processing, including in the legacy decrypt interface. */
void AES_ECB_encrypt_buffer(const struct nv_aes_context *,unsigned char *,int length);
void AES_ECB_decrypt_buffer(const struct nv_aes_context *,unsigned char *,int length);
#endif
