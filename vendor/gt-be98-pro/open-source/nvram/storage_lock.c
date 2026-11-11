/* SPDX-License-Identifier: GPL-2.0-only */
#define _POSIX_C_SOURCE 200809L
#include "storage_lock.h"
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
int nv_storage_lock_acquire(int directory,unsigned attempts,unsigned delay_ms) {
    if(directory<0 || !attempts || attempts>1000 || delay_ms>1000) {errno=EINVAL;return -1;}
    for(unsigned i=0;i<attempts;++i) {
        int fd=openat(directory,"nvram.lock",O_WRONLY|O_CREAT|O_EXCL|O_CLOEXEC|O_NOFOLLOW,0644);
        if(fd>=0) return fd;
        if(errno!=EEXIST || i+1==attempts) return -1;
        struct timespec delay={delay_ms/1000,(long)(delay_ms%1000)*1000000};
        while(nanosleep(&delay,&delay)) if(errno!=EINTR) return -1;
    }
    errno=EEXIST;return -1;
}
int nv_storage_lock_release(int directory,int owned_fd) {
    struct stat owned,current;int result=-1,saved;
    if(directory<0 || owned_fd<0) {errno=EINVAL;return -1;}
    if(!fstat(owned_fd,&owned) && !fstatat(directory,"nvram.lock",&current,AT_SYMLINK_NOFOLLOW)) {
        if(owned.st_dev!=current.st_dev || owned.st_ino!=current.st_ino) errno=ESTALE;
        else result=unlinkat(directory,"nvram.lock",0);
    }
    saved=errno;
    if(close(owned_fd) && !result) return -1;
    errno=saved;return result;
}
