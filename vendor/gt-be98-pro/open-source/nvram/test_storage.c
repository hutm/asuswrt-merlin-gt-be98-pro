/* SPDX-License-Identifier: GPL-2.0-only */
#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include "storage_keys.h"
int large_nvram(const char *);int large_nvram_t(const char *);
char *jffs_nvram_get(const char *);int jffs_nvram_set(const char *,const char *);int jffs_nvram_unset(const char *);
static int directory=1,marker=1,present=1,read_result=5,write_result=17,unlink_result=0;
static char last_path[4096];
int d_exists(const char *path){assert(!strcmp(path,"/jffs") || !strcmp(path,"/jffs/nvram"));return directory;}
int f_exists(const char *path){if(!strcmp(path,"/jffs/nvram_war"))return marker;snprintf(last_path,sizeof last_path,"%s",path);return present;}
int mkdir(const char *path,mode_t mode){assert(!strcmp(path,"/jffs/nvram") && mode==0777);return 0;}
int unlink(const char *path){snprintf(last_path,sizeof last_path,"%s",path);return unlink_result;}
int f_read_string(const char *path,char *out,int capacity){assert(capacity==16384);snprintf(last_path,sizeof last_path,"%s",path);if(read_result>0)strcpy(out,"value");return read_result;}
int f_write_string(const char *path,const char *value,int flags,unsigned mode){assert(!strcmp(value,"value") && flags==0 && mode==0);snprintf(last_path,sizeof last_path,"%s",path);return write_result;}
char *wlcsm_nvram_get(const char *key){(void)key;return NULL;}
int nvram_set(const char *key,const char *value){(void)key;(void)value;return 0;}
int nvram_commit(void){return 0;}
int main(void){
 size_t count=0;
 for(int d=0;d<=1;++d)for(int m=0;m<=1;++m){directory=d;marker=m;
  for(size_t i=0;large_nvram_list[i];++i)assert(large_nvram(large_nvram_list[i])==(d&&m));
 }
 directory=marker=1;
 for(size_t i=0;large_nvram_list[i];++i){char suffix[256];snprintf(suffix,sizeof suffix,"%s_not_a_key",large_nvram_list[i]);assert(!large_nvram(suffix));++count;}
 assert(!large_nvram("unknown_key") && !large_nvram(""));
 for(size_t i=0;large_nvram_list_t[i];++i)assert(large_nvram_t(large_nvram_list_t[i]));
 assert(!large_nvram_t("unknown_key"));
 assert(!strcmp(jffs_nvram_get("key"),"value") && !strcmp(last_path,"/jffs/nvram/key"));
 present=0;assert(jffs_nvram_get("key")==NULL);present=1;
 read_result=0;assert(!strcmp(jffs_nvram_get("key"),""));
 read_result=-1;assert(!strcmp(jffs_nvram_get("key"),""));
 assert(jffs_nvram_set("key","value")==17 && !strcmp(last_path,"/jffs/nvram/key"));
 write_result=-1;assert(jffs_nvram_set("key","value")==-1);
 unlink_result=-1;assert(jffs_nvram_set("key",NULL)==-1 && jffs_nvram_unset("key")==-1);
 assert(jffs_nvram_get(NULL)==NULL && errno==EINVAL);
 assert(jffs_nvram_get("../outside")==NULL && errno==EINVAL);
 assert(jffs_nvram_set(".","value")==-1 && errno==EINVAL);
 assert(jffs_nvram_unset("/")==-1 && errno==EINVAL);
 {char oversized[4096];memset(oversized,120,sizeof oversized-1);oversized[sizeof oversized-1]=0;
  assert(jffs_nvram_set(oversized,"value")==-1 && errno==ENAMETOOLONG);}
 printf("Storage compatibility: %zu routing keys, marker gates, suffix rejection, migration list, get/set/unset return behavior passed\n",count);
 return 0;
}
