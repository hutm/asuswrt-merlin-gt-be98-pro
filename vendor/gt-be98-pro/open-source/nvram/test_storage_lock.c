/* SPDX-License-Identifier: GPL-2.0-only */
#define _POSIX_C_SOURCE 200809L
#include "storage_lock.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
int main(void) {
    char path[]="/tmp/nv-lock-XXXXXX",byte;int directory,fd,p[2],q[2],status;
    assert(mkdtemp(path));directory=open(path,O_RDONLY|O_DIRECTORY);assert(directory>=0);
    fd=nv_storage_lock_acquire(directory,1,0);assert(fd>=0);
    assert(fcntl(fd,F_GETFD)&FD_CLOEXEC);
    assert(nv_storage_lock_acquire(directory,2,1)==-1&&errno==EEXIST);
    assert(!pipe(p)&&!pipe(q));pid_t child=fork();assert(child>=0);
    if(!child) {
        close(fd);close(p[1]);close(q[0]);
        assert(nv_storage_lock_acquire(directory,1,0)==-1&&errno==EEXIST);
        assert(write(q[1],"x",1)==1);
        assert(read(p[0],&byte,1)==1);
        int own=nv_storage_lock_acquire(directory,1,0);assert(own>=0);
        assert(!nv_storage_lock_release(directory,own));_exit(0);
    }
    close(p[0]);close(q[1]);assert(read(q[0],&byte,1)==1);
    assert(!nv_storage_lock_release(directory,fd));assert(write(p[1],"x",1)==1);
    close(p[1]);close(q[0]);assert(waitpid(child,&status,0)==child&&WIFEXITED(status)&&WEXITSTATUS(status)==0);
    /* Do not remove a lock pathname replaced since acquisition. */
    fd=nv_storage_lock_acquire(directory,1,0);assert(fd>=0);assert(!unlinkat(directory,"nvram.lock",0));
    int replacement=openat(directory,"nvram.lock",O_CREAT|O_EXCL|O_WRONLY,0644);assert(replacement>=0);close(replacement);
    assert(nv_storage_lock_release(directory,fd)==-1&&errno==ESTALE);
    assert(!faccessat(directory,"nvram.lock",F_OK,0));assert(!unlinkat(directory,"nvram.lock",0));
    assert(!symlinkat("untouched",directory,"nvram.lock"));
    assert(nv_storage_lock_acquire(directory,1,0)==-1&&errno==EEXIST);
    assert(!unlinkat(directory,"nvram.lock",0));
    assert(nv_storage_lock_acquire(directory,0,0)==-1&&errno==EINVAL);
    close(directory);assert(!rmdir(path));
    puts("Storage lock: exclusive creation, process exclusion, release, replaced pathname and symlink refusal passed");
    return 0;
}
