/* SPDX-License-Identifier: GPL-2.0-only */
#include "migration.h"
#include "settings.h"
#include <errno.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#if UINTPTR_MAX == UINT32_MAX
_Static_assert(sizeof(struct nvram_tuple)==20,"ARM tuple size");
_Static_assert(offsetof(struct nvram_tuple,enc)==14,"ARM encryption flag offset");
#endif
#include <openssl/crypto.h>
extern struct nvram_tuple router_defaults[];
extern char *nvram_get(const char *);
extern int nvram_set(const char *,const char *);
struct pending {const char *name;char value[4096];struct pending *next;};
static int migrate(int decrypt)
{
 struct pending *head=NULL,**tail=&head;int status=-1;
 for(struct nvram_tuple *t=router_defaults;t->name;++t){
  if(t->enc!=1)continue;
  struct pending *p=calloc(1,sizeof *p);if(!p)goto done;
  *tail=p;tail=&p->next;p->name=t->name;
  const char *value=nvram_get(t->name);
  int result=decrypt?dec_nvram(t->name,(char *)(value?value:""),p->value):enc_nvram(t->name,(char *)(value?value:""),p->value);
  if(result<0)goto done;
 }
 for(struct pending *p=head;p;p=p->next){
  if(nvram_set(p->name,p->value)!=0){errno=EIO;goto done;}
 }
 if(nvram_set("nvram_encrypt",decrypt?"0":"1")!=0){errno=EIO;goto done;}
 status=0;
done:
 {int error=errno;while(head){struct pending *next=head->next;OPENSSL_clear_free(head,sizeof *head);head=next;}errno=error;}
 return status;
}
int start_enc_nvram(void){return migrate(0);}
int start_dec_nvram(void){return migrate(1);}
