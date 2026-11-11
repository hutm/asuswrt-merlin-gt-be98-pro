/* SPDX-License-Identifier: GPL-2.0-only */
/* Experimental GT-BE98_PRO JFFS storage hooks. External serialization required:
 * successful reads share one buffer, overwritten by the next read. */
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "storage_keys.h"
extern int d_exists(const char *);
extern int f_exists(const char *);
extern int f_read_string(const char *,char *,int);
extern int f_write_string(const char *,const char *,int,unsigned);
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
