#define _FILE_OFFSET_BITS 64
#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
int asusdebuglog(int, char *, int, int, unsigned, const char *, ...);
int asusdebuglog_file_lock(int);
int asusdebuglog_file_unlock(int);
char *nvram_get(const char *key) { (void)key;return NULL; }
static void succeeds(char *path) {
    assert(asusdebuglog(6,path,0,0,1,"after error\n")==1);
}
int main(void) {
    char tmp[]="/tmp/openlog-errors-XXXXXX",path[256],lockpath[256],shared[128];
    int fd,status;
    int32_t owner;
    pid_t child;
    struct stat st;
    struct rlimit old,limited;
    assert(mkdtemp(tmp));
    snprintf(path,sizeof path,"%s/log",tmp);
    /* Buffered writes can fail only when fclose flushes them. */
    assert(asusdebuglog(6,"/dev/full",0,0,1,"full device\n")==0);
    succeeds(path);
    assert(asusdebuglog(6,tmp,0,0,1,"directory\n")==0);
    succeeds(path);
    assert(asusdebuglog(6,NULL,0,0,1,"no path\n")==-1);
    succeeds(path);
    assert(asusdebuglog_file_lock(-1)==-1);
    assert(asusdebuglog_file_unlock(-1)==-1 && errno==EBADF);
    snprintf(lockpath,sizeof lockpath,"%s/lock",tmp);
    fd=open(lockpath,O_RDWR|O_CREAT,0600);assert(fd>=0);
    assert(asusdebuglog_file_lock(fd)==0);
    assert(lseek(fd,0,SEEK_SET)==0);
    assert(read(fd,&owner,sizeof owner)==sizeof owner && owner==(int32_t)getpid());
    child=fork();assert(child>=0);
    if(child==0) {
        struct flock lock={0};
        lock.l_type=F_WRLCK;lock.l_whence=SEEK_SET;
        assert(fcntl(fd,F_GETLK,&lock)==0);
        assert(lock.l_type==F_WRLCK && lock.l_pid==getppid());
        _exit(0);
    }
    assert(waitpid(child,&status,0)==child && WIFEXITED(status) && WEXITSTATUS(status)==0);
    assert(asusdebuglog_file_unlock(fd)==0);
    assert(fcntl(fd,F_GETFD)==-1 && errno==EBADF);
    assert(stat(lockpath,&st)==0 && st.st_size==0);
    /* Failure to acquire the internal lock must release the thread mutex. */
    assert(getrlimit(RLIMIT_NOFILE,&old)==0);limited=old;limited.rlim_cur=3;
    assert(setrlimit(RLIMIT_NOFILE,&limited)==0);
    assert(asusdebuglog(6,path,0,0,1,"no descriptors\n")==-1);
    assert(setrlimit(RLIMIT_NOFILE,&old)==0);
    succeeds(path);
    /* The shared lock must not follow a symlink planted by another process. */
    snprintf(shared,sizeof shared,"/tmp/open-asusdebuglog-%ld.lock",(long)geteuid());
    assert(unlink(shared)==0);
    assert(symlink(path,shared)==0);
    assert(asusdebuglog(6,path,0,0,1,"symlink\n")==-1);
    assert(unlink(shared)==0);
    succeeds(path);
    unlink(path);unlink(lockpath);rmdir(tmp);
    puts("logging failures: flush/open errors, lock ownership, descriptor exhaustion and symlink rejection passed");
    return 0;
}
