/* SPDX-License-Identifier: GPL-2.0-only */
#ifndef OPEN_NVRAM_PERSIST_H
#define OPEN_NVRAM_PERSIST_H
#include "settings.h"
#include <fcntl.h>
#include <unistd.h>
/* Caller must serialize all writers and supply a trusted directory FD.
 * 0: installed and synced; -1: failed before installation; -2: installed but
 * directory sync failed (durability uncertain). Notification is a separate step.
 * Existing _new is never overwritten: boot recovery must resolve it first. */
static int nv_persist_at(int directory,const void *data,size_t size) {
    const char *main=".kernel_nvram.setting",*next=".kernel_nvram.setting_new";
    const char *old=".kernel_nvram.setting_old";
    FILE *stream=NULL;
    int fd=-1,result=-1,saved,owned=0,moved=0,installed=0;
    fd=openat(directory,next,O_WRONLY|O_CREAT|O_EXCL|O_NOFOLLOW|O_CLOEXEC,0600);
    if(fd<0) return -1;
    owned=1;
    stream=fdopen(fd,"w");
    if(!stream) goto done;
    if(nv_settings_write(stream,data,size) || fflush(stream) || fsync(fd)) goto done;
    if(fclose(stream)) { stream=NULL;fd=-1;goto done; }
    stream=NULL;fd=-1;
    /* Persist the complete recovery file before moving the current file. */
    if(fsync(directory)) goto done;
    if(renameat(directory,main,directory,old)) {
        if(errno!=ENOENT) goto done;
    } else moved=1;
    if(renameat(directory,next,directory,main)) goto done;
    owned=0;installed=1;
    if(fsync(directory)) { result=-2;goto done; }
    result=0;
done:
    saved=errno;
    if(stream) fclose(stream);else if(fd>=0) close(fd);
    if(result && !installed) {
        /* Restore the live name if installation failed after moving it aside.
         * If rollback fails, retain _new and _old for existing boot recovery. */
        if(moved && renameat(directory,old,directory,main)) owned=0;
        if(owned) unlinkat(directory,next,0);
        (void)fsync(directory);
    }
    errno=saved;return result;
}
#endif
