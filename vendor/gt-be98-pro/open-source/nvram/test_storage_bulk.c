/* SPDX-License-Identifier: GPL-2.0-only */
#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <errno.h>
#include <stdlib.h>
int internal_nvram_get_int(const char *);void jffs_nvram_init(void);
int jffs_nvram_getall(int,char *,int);
static int directory=1,special_done,writes,sets,commits,fail_write;
static const char *keys[]={"MULTIFILTER_MAC","cfg_relist","cfg_relist_x"};
int d_exists(const char *path){assert(!strcmp(path,"/jffs") || !strcmp(path,"/jffs/nvram"));return directory;}
int f_exists(const char *path){for(int i=0;i<3;++i)if(!strcmp(path+12,keys[i]))return 1;return 0;}
int mkdir(const char *path,mode_t mode){(void)path;(void)mode;return 0;}
int unlink(const char *path){(void)path;return 0;}
int f_read_string(const char *path,char *out,int capacity){(void)path;assert(capacity==16384);strcpy(out,"value");return 5;}
int f_write_string(const char *path,const char *value,int flags,unsigned mode){
 assert(!strcmp(path,"/jffs/nvram/MULTIFILTER_MAC") || !strcmp(path,"/jffs/nvram/cfg_relist") || !strcmp(path,"/jffs/nvram/cfg_relist_x"));
 assert(!strcmp(value,"value") && !flags && !mode);++writes;return fail_write?-1:5;
}
char *wlcsm_nvram_get(const char *key){
 for(int i=0;i<3;++i)if(!strcmp(key,keys[i]))return "value";
 if(!strcmp(key,"cfg_relist_t") || !strcmp(key,"cfg_relist_x_t"))return special_done?"1":"0";
 if(!strcmp(key,"number"))return " -12tail";
 return NULL;
}
int nvram_set(const char *key,const char *value){
 if(!strcmp(key,keys[0]))assert(!*value);
 else {assert(!strcmp(key,"cfg_relist_t") || !strcmp(key,"cfg_relist_x_t"));assert(!strcmp(value,"1"));}
 ++sets;return 0;
}
int nvram_commit(void){++commits;return 0;}
int main(void){
 char out[256];int expected=0;
 assert(internal_nvram_get_int("missing")==0 && internal_nvram_get_int("number")==-12);
 directory=0;jffs_nvram_init();assert(!writes && !sets && !commits);directory=1;
 jffs_nvram_init();assert(writes==3 && sets==3 && commits==1);
 writes=sets=commits=0;special_done=1;jffs_nvram_init();assert(writes==1 && sets==1 && commits==1);
 memset(out,0x55,sizeof out);memcpy(out,"prefix\0",7);
 int result=jffs_nvram_getall(7,out,sizeof out);expected=7;
 for(int i=0;i<3;++i){char record[100];int n=snprintf(record,sizeof record,"%s=value",keys[i])+1;assert(!memcmp(out+expected,record,n));expected+=n;}
 assert(result==expected && out[result]==0x55 && !memcmp(out,"prefix\0",7));
 /* Exact record-sized remainder must be skipped; with one extra byte it fits. */
 int size=(int)strlen(keys[0])+7;
 memset(out,0x55,sizeof out);result=jffs_nvram_getall(0,out,size);
 assert(result>0 && strncmp(out,keys[0],strlen(keys[0]))); /* smaller later entry fits */
 memset(out,0x55,sizeof out);result=jffs_nvram_getall(0,out,size+1);
 assert(result==size && !strncmp(out,keys[0],strlen(keys[0])) && out[size]==0x55);
#ifndef REFERENCE_OBJECT
 writes=sets=commits=0;fail_write=1;jffs_nvram_init();assert(writes==1 && !sets && !commits);
 assert(jffs_nvram_getall(-1,out,sizeof out)==-1 && errno==EINVAL);
 assert(jffs_nvram_getall(0,NULL,sizeof out)==-1 && errno==EINVAL);
#endif
 puts("Storage bulk: integer lookup, migration gating/flags, ordered enumeration and capacity boundaries passed");return 0;
}
