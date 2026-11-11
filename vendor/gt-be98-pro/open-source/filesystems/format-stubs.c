#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define dbg(...) ((void)0)
#define logmessage(...) ((void)0)
#define DISKMON_UMOUNT 1
#define DISKMON_FORMAT 2
#define DISKMON_REMOUNT 3
#define DISKMON_FINISH 4
#define EFH_HP_REMOVE 1
#define EFH_USER 2
#define EFH_HP_ADD 4
#define O_RDONLY 0
#define BLKRRPART 0
struct partition {char *device;struct partition *next;};
struct disk {char *port,*device;struct partition *partitions;struct disk *next;};
typedef struct partition partition_info_t;
typedef struct disk disk_info_t;
static struct disk disk={"1","sda",NULL,NULL};
static int mount_counter,missing_disk,format_status,format_calls,nas_calls;
static const char *system_name="tfat",*mode="open";
static char final_flag[8];
static void nvram_set(const char *key,const char *value) {if(!strcmp(key,"disk_format_flag")) snprintf(final_flag,sizeof final_flag,"%s",value);}
static const char *nvram_safe_get(const char *key) {
 if(!strcmp(key,"diskformat_file_system")) return system_name;
 if(!strcmp(key,"diskformat_label")) return "label with spaces;literal";
 return "1";
}
static int nvram_match(const char *key,const char *value) {(void)key;return !strcmp(mode,value);}
static size_t strlcpy(char *to,const char *from,size_t size) {snprintf(to,size,"%s",from);return strlen(from);}
static int safe_atoi(const char *s) {return atoi(s);}
static disk_info_t *read_disk_data(void) {return missing_disk?NULL:&disk;}
static void free_disk_data(disk_info_t **p) {(void)p;}
static void stop_nas_services(int n) {(void)n;}
static void start_nas_services(int n) {(void)n;++nas_calls;}
static void diskmon_status(int n) {(void)n;}
#define umount_partition(...) ((void)0)
#define findmntents(...) ((void)0)
#define check_if_dir_exist(...) 1
#define mkdir(...) 0
#define check_if_file_exist(...) 0
#define unlink(...) 0
#define eval(...) 0
#define sync(...) ((void)0)
#define open(...) (-1)
#define ioctl(...) 0
#define close(...) 0
#define sleep(...) ((void)0)
#define mount_partition(...) 0
static time_t uptime(void) {return 1;}
static FILE *test_fopen(const char *path,const char *mode_arg) {(void)mode_arg;assert(!strcmp(path,"/proc/partitions"));return tmpfile();}
#define fopen test_fopen
static int _eval(char **cmd,const char *file,int n,void *p) {
 int labels=0,devices=0;(void)file;(void)n;(void)p;++format_calls;
 for(int i=0;cmd[i];++i) {labels+=!strcmp(cmd[i],"label with spaces;literal");devices+=!strcmp(cmd[i],"/dev/sda");}
 assert(labels==1 && devices==1);
 if(!strcmp(system_name,"tfat")) assert(!strcmp(cmd[0],"mkfs.vfat"));
 if(!strcmp(system_name,"tntfs")) assert(!strcmp(cmd[0],"mkntfs"));
 if(!strcmp(system_name,"thfsplus")) assert(!strcmp(cmd[0],"mkfs.hfsplus"));
 return format_status;
}
/* FUNCTION_INSERT */
int main(void) {
 const char *types[]={"tfat","tntfs","thfsplus"};
 for(int i=0;i<3;++i) for(int fail=0;fail<2;++fail) {
  system_name=types[i];format_status=fail;format_calls=nas_calls=0;
  start_diskformat("1");assert(format_calls==1 && nas_calls==1);
  assert(!strcmp(final_flag,fail?"-1":"4"));
 }
 format_calls=0;system_name="unsupported";start_diskformat("1");assert(!format_calls && !strcmp(final_flag,"-1"));
 disk.port="2";start_diskformat("1");assert(!format_calls && !strcmp(final_flag,"-1"));
 missing_disk=1;start_diskformat("1");assert(!format_calls && !strcmp(final_flag,"-1"));
 puts("Full format function: commands, failure reporting, missing disks and literal labels passed");return 0;
}
