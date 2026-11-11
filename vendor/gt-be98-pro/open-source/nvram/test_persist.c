#define _GNU_SOURCE
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
static int sync_calls,rename_calls,fail_sync,fail_rename,fail_write,fail_rollback;
static size_t test_write(const void *data,size_t size,size_t count,FILE *stream) {
    if(fail_write) { errno=ENOSPC;return 0; }return fwrite(data,size,count,stream);
}
static int test_sync(int fd) {
    if(++sync_calls==fail_sync) { errno=EIO;return -1; }return fsync(fd);
}
static int test_rename(int a,const char *b,int c,const char *d) {
    ++rename_calls;
    if(rename_calls==fail_rename || (fail_rollback && rename_calls==3)) {
        errno=EIO;return -1;
    }
    return renameat(a,b,c,d);
}
#define fwrite test_write
#define fsync test_sync
#define renameat test_rename
#include "persist.h"
#undef fwrite
#undef fsync
#undef renameat
static int unused_apply(void *c,const char *n,const char *v) { (void)c;(void)n;(void)v;return 0; }
static void check(int dir,const char *name,const char *expected) {
    char bytes[100]={0};int fd=openat(dir,name,O_RDONLY);assert(fd>=0);
    assert(read(fd,bytes,sizeof bytes-1)>=0);close(fd);assert(!strcmp(bytes,expected));
}
int main(void) {
    char path[]="/tmp/persist-test-XXXXXX";
    const char data[]="new=value\0empty=\0";
    int dir,fd,result;
    assert(mkdtemp(path));dir=open(path,O_DIRECTORY|O_RDONLY);assert(dir>=0);
    /* Exercise loader too so this header-only test is warning-clean. */
    FILE *empty=tmpfile();assert(empty);assert(!nv_settings_load(empty,unused_apply,NULL));fclose(empty);
    for(int scenario=0;scenario<8;++scenario) {
        unlinkat(dir,".kernel_nvram.setting",0);unlinkat(dir,".kernel_nvram.setting_old",0);unlinkat(dir,".kernel_nvram.setting_new",0);
        fd=openat(dir,".kernel_nvram.setting",O_WRONLY|O_CREAT|O_TRUNC,0600);assert(fd>=0);
        assert(write(fd,"previous\n",9)==9);close(fd);
        sync_calls=rename_calls=0;fail_sync=fail_rename=fail_write=fail_rollback=0;
        if(scenario>=1 && scenario<=3) fail_sync=scenario;
        if(scenario>=4 && scenario<=5) fail_rename=scenario-3;
        if(scenario==6) fail_write=1;
        if(scenario==7) {fail_rename=2;fail_rollback=1;}
        result=nv_persist_at(dir,data,sizeof data);
        if(!scenario || scenario==3) {
            assert(result==(scenario==3?-2:0));
            check(dir,".kernel_nvram.setting","new=value\nempty=\n");
            check(dir,".kernel_nvram.setting_old","previous\n");
        } else if(scenario==7) {
            assert(result==-1);
            assert(faccessat(dir,".kernel_nvram.setting",F_OK,0)==-1 && errno==ENOENT);
            check(dir,".kernel_nvram.setting_old","previous\n");
            check(dir,".kernel_nvram.setting_new","new=value\nempty=\n");
        } else { assert(result==-1);check(dir,".kernel_nvram.setting","previous\n"); }
        if(scenario!=7)
            assert(faccessat(dir,".kernel_nvram.setting_new",F_OK,0)==-1 && errno==ENOENT);
    }
    fail_sync=fail_rename=fail_write=fail_rollback=0;
    unlinkat(dir,".kernel_nvram.setting_new",0);
    fd=openat(dir,".kernel_nvram.setting_new",O_WRONLY|O_CREAT|O_EXCL,0600);assert(fd>=0);
    assert(write(fd,"recovery\n",9)==9);close(fd);
    assert(nv_persist_at(dir,data,sizeof data)==-1 && errno==EEXIST);
    check(dir,".kernel_nvram.setting_new","recovery\n");
    unlinkat(dir,".kernel_nvram.setting",0);unlinkat(dir,".kernel_nvram.setting_old",0);unlinkat(dir,".kernel_nvram.setting_new",0);
    close(dir);rmdir(path);
    puts("Persistence: backup, write/sync/rename failure states and existing recovery-file preservation passed");
    return 0;
}
