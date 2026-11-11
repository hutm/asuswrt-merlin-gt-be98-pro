/* SPDX-License-Identifier: GPL-2.0-only */
/* Experimental GT-BE98_PRO JFFS storage hooks. External serialization required:
 * successful reads share one buffer, overwritten by the next read. */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "storage_keys.h"
extern int d_exists(const char *);
extern int f_exists(const char *);
extern int f_read_string(const char *,char *,int);
extern int f_write_string(const char *,const char *,int,unsigned);
extern char *wlcsm_nvram_get(const char *);
extern int nvram_set(const char *,const char *);
extern int nvram_commit(void);
static char value_buffer[16384];
static int make_path(const char *key,char *path,size_t size) {
    int length;
    if(!key || !*key || strchr(key,'/') || !strcmp(key,".") || !strcmp(key,"..")) {
        errno=EINVAL;return -1;
    }
    length=snprintf(path,size,"/jffs/nvram/%s",key);
    if(length<0 || (size_t)length>=size) {errno=ENAMETOOLONG;return -1;}
    return 0;
}
static void ensure_directory(void) {
    if(!d_exists("/jffs/nvram")) (void)mkdir("/jffs/nvram",0777);
}
char *jffs_nvram_get(const char *key) {
    char path[4096];
    if(make_path(key,path,sizeof path)) return NULL;
    memset(value_buffer,0,sizeof value_buffer);ensure_directory();
    if(!f_exists(path)) return NULL;
    if(f_read_string(path,value_buffer,sizeof value_buffer)>0) return value_buffer;
    return "";
}
int jffs_nvram_unset(const char *key) {
    char path[4096];
    if(make_path(key,path,sizeof path)) return -1;
    ensure_directory();return unlink(path);
}
int jffs_nvram_set(const char *key,const char *value) {
    char path[4096];
    if(!value) return jffs_nvram_unset(key);
    if(make_path(key,path,sizeof path)) return -1;
    ensure_directory();return f_write_string(path,value,0,0);
}
int large_nvram(const char *key) {
    if(!key || !d_exists("/jffs") || !f_exists("/jffs/nvram_war")) return 0;
    for(size_t i=0;large_nvram_list[i];++i)
        if(!strcmp(key,large_nvram_list[i])) return 1;
    return 0;
}
int large_nvram_t(const char *key) {
    if(!key) return 0;
    for(size_t i=0;large_nvram_list_t[i];++i)
        if(!strcmp(key,large_nvram_list_t[i])) return 1;
    return 0;
}
int internal_nvram_get_int(const char *key) {
    const char *value=wlcsm_nvram_get(key);return atoi(value?value:"");
}
void jffs_nvram_init(void) {
    int changed=0;
    if(!d_exists("/jffs")) return;
    for(size_t i=0;large_nvram_list[i];++i) {
        const char *key=large_nvram_list[i];
        char *value=wlcsm_nvram_get(key),flag[256];int special=large_nvram_t(key),written;
        if(!value || !*value) continue;
        if(snprintf(flag,sizeof flag,"%s_t",key)>=(int)sizeof flag) {errno=ENAMETOOLONG;return;}
        if(special && internal_nvram_get_int(flag)) continue;
        /* Keep the kernel value if the file copy failed or was short. */
        size_t length=strlen(value);
        written=jffs_nvram_set(key,value);
        if(written<0 || (size_t)written!=length) {errno=EIO;return;}
        if(nvram_set(special?flag:key,special?"1":"")) return;
        changed=1;
    }
    if(changed) (void)nvram_commit();
}
int jffs_nvram_getall(int used,char *output,int capacity) {
    if(!output || used<0 || capacity<0 || used>capacity) {errno=EINVAL;return -1;}
    for(size_t i=0;large_nvram_list[i];++i) {
        const char *key=large_nvram_list[i];char *value=jffs_nvram_get(key);
        size_t key_length,value_length,length;
        if(!value) continue;
        key_length=strlen(key);value_length=strlen(value);length=key_length+value_length+2;
        /* Preserve the existing strict extra-byte capacity rule and skip policy. */
        if(length>=(size_t)(capacity-used)) continue;
        memcpy(output+used,key,key_length);output[used+key_length]='=';
        memcpy(output+used+key_length+1,value,value_length+1);used+=(int)length;
    }
    return used;
}
