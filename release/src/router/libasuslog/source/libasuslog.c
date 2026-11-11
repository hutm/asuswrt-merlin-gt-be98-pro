/* SPDX-License-Identifier: GPL-2.0-only */
/* Source implementation selected only by the experimental GT-BE98_PRO profile. */
#define _FILE_OFFSET_BITS 64
#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <syslog.h>
#include <time.h>
#include <unistd.h>

extern char *nvram_get(const char *);
int log_num;
static pthread_mutex_t log_mutex = PTHREAD_MUTEX_INITIALIZER;

char *my_strrchr(const char *s, int c)
{
    /* The vendor helper searches non-NUL bytes only. */
    const char *last = NULL;
    if (!s) { errno = EINVAL; return NULL; }
    for (; *s; ++s) if ((unsigned char)*s == c) last = s;
    return (char *)last;
}

void my_mkdir(const char *path)
{
    char *copy, *p;
    if (!path || !*path) return;
    copy = strdup(path);
    if (!copy) return;
    for (p = copy + 1; *p; ++p) {
        if (*p != '/') continue;
        *p = 0;
        (void)mkdir(copy, 0777);
        *p = '/';
    }
    (void)mkdir(copy, 0777);
    free(copy);
}

int asusdebuglog_file_lock(int fd)
{
    struct flock lock = {0};
    int32_t pid = (int32_t)getpid();
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    while (fcntl(fd, F_SETLKW, &lock) < 0) {
        if (errno != EINTR) return -1;
    }
    if (lseek(fd, 0, SEEK_SET) < 0 || write(fd, &pid, sizeof(pid)) != sizeof(pid))
        return -1;
    return 0;
}

int asusdebuglog_file_unlock(int fd)
{
    if (fd < 0) { errno = EBADF; return -1; }
    (void)ftruncate(fd, 0);
    return close(fd);
}

int unlink_asusdebuglog_file_lock(const char *path)
{
    if (path) (void)unlink(path);
    return 0;
}

int rm_asusdebuglog(char *path)
{
    (void)unlink(path);
    return 0;
}

int asusdebuglog(int level, char *path, int conlog, int showtime,
                 unsigned filesize, const char *msgfmt, ...)
{
    static const char *levels[] = {"emergency", "alert", "critical", "error",
                                  "warning", "notice", "info", "debug"};
    char message[1024], filename[PATH_MAX], rotated[PATH_MAX], timestamp[32];
    char lockname[128];
    const char *base;
    struct stat st;
    size_t limit;
    FILE *out = NULL;
    int fd = -1, result = -1, n, custom, did_rotate = 0;
    va_list args;
    time_t now;
    struct tm local;
    if (conlog == -1) return 1;
    if (!msgfmt || level < 0 || level > 7) { errno = EINVAL; return -1; }
    va_start(args, msgfmt);
    n = vsnprintf(message, sizeof(message), msgfmt, args);
    va_end(args);
    if (n < 0) return -1;
    pthread_mutex_lock(&log_mutex);
    log_num = 0;
    snprintf(lockname, sizeof(lockname), "/tmp/open-asusdebuglog-%ld.lock", (long)geteuid());
    fd = open(lockname, O_RDWR | O_CREAT | O_CLOEXEC | O_NOFOLLOW, 0600);
    if (fd < 0 || asusdebuglog_file_lock(fd) < 0) goto done;
    if (conlog == 1) {
        syslog(level ? level : LOG_INFO, "%s", message);
        result = 1;
        goto done;
    }
    if (conlog == 2) {
        out = fopen("/dev/console", "w");
        if (!out) { result = 0; goto done; }
        result = fprintf(out, "\n (%s) \n", message) < 0 ? 0 : 1;
        goto done;
    }
    if (!path || !*path) { errno = EINVAL; goto done; }
    base = nvram_get("asuslog_path");
    custom = base && *base;
    if (!custom) base = "/tmp/asusdebuglog";
    limit = custom ? (filesize ? (size_t)filesize * 1024 : 2097152) :
                    (filesize ? (filesize > 1024 ? 1048576 : (size_t)filesize * 1024) : 204800);
    if (strchr(path, '/')) n = snprintf(filename, sizeof(filename), "%s", path);
    else {
        my_mkdir(base);
        n = snprintf(filename, sizeof(filename), "%s/%s", base, path);
    }
    if (n < 0 || (size_t)n >= sizeof(filename)) { errno = ENAMETOOLONG; goto done; }
    out = fopen(filename, "a");
    if (!out && custom) {
        base = "/tmp/asusdebuglog";
        my_mkdir(base);
        n = snprintf(filename, sizeof(filename), "%s/%s", base, path);
        if (n < 0 || (size_t)n >= sizeof(filename)) { errno = ENAMETOOLONG; goto done; }
        limit = filesize ? (filesize > 1024 ? 1048576 : (size_t)filesize * 1024) : 204800;
        out = fopen(filename, "a");
    }
    if (!out) { result = 0; goto done; }
    if (stat(filename, &st) == 0 && (uint64_t)st.st_size >= limit) {
        if (fclose(out) != 0) { out = NULL; result = 0; goto done; }
        out = NULL;
        log_num = custom ? -1 : 0;
        do {
            if (log_num == INT_MAX) { errno = EOVERFLOW; goto done; }
            ++log_num;
            n = snprintf(rotated, sizeof(rotated), "%s-%d", filename, log_num);
            if (n < 0 || (size_t)n >= sizeof(rotated)) { errno = ENAMETOOLONG; goto done; }
        } while (custom && access(rotated, F_OK) == 0);
        if (rename(filename, rotated) < 0) { result = 0; goto done; }
        did_rotate = 1;
    }
    if (!out) out = fopen(filename, "a");
    if (!out) { result = 0; goto done; }
    if (did_rotate) { result = fputs(message, out) < 0 ? 0 : 1; goto done; }
    if (showtime || custom) {
        now = time(NULL);
        if (!localtime_r(&now, &local) ||
            !strftime(timestamp, sizeof(timestamp), "%a %b %e %H:%M:%S %Y", &local))
            goto done;
        if (fprintf(out, "%s  ", timestamp) < 0) { result = 0; goto done; }
    }
    if (!showtime && !custom && fputc(' ', out) == EOF) { result = 0; goto done; }
    if (custom && fprintf(out, "[pid=%ld] ", (long)getpid()) < 0) { result = 0; goto done; }
    if (level && fprintf(out, "[%s] ", levels[level]) < 0) { result = 0; goto done; }
    result = fputs(message, out) < 0 ? 0 : 1;
done:
    if (out && fclose(out) != 0) result = 0;
    if (fd >= 0) (void)asusdebuglog_file_unlock(fd);
    /* Keep the shared lock inode stable while other processes may wait on it. */
    pthread_mutex_unlock(&log_mutex);
    return result;
}
