#define _GNU_SOURCE
#define _FILE_OFFSET_BITS 64
#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <dirent.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
typedef int (*logger)(int,char *,int,int,unsigned,const char *,...);
time_t time(time_t *out) { time_t value=1700000000; if(out)*out=value; return value; }
/* Capture external logging sinks without accessing the host console/syslog. */
void syslog(int priority, const char *format, ...) {
    va_list args;
    printf("SYSLOG priority=%d: ", priority);
    va_start(args, format); vprintf(format, args); va_end(args);
    puts("ENDSYSLOG");
}
FILE *fopen64(const char *path, const char *mode) {
    FILE *(*real_open)(const char *, const char *) = dlsym(RTLD_NEXT, "fopen64");
    assert(real_open);
    return real_open(!strcmp(path,"/dev/console") ? "/tmp/log-compare/console" : path, mode);
}
int cprintf(const char *format, ...) {
    FILE *f=fopen("/dev/console","w");
    va_list args;
    int result;
    if(!f) return -1;
    va_start(args,format);result=vfprintf(f,format,args);va_end(args);
    fclose(f);return result;
}
static void show(const char *name) {
    char path[256],buf[4096];
    FILE *f;
    snprintf(path,sizeof(path),"/tmp/log-compare/%s",name);
    f=fopen(path,"r");
    printf("FILE %s: ",name);
    if(!f) { puts("missing");return; }
    while(fgets(buf,sizeof(buf),f)) {
        char *pid=strstr(buf,"[pid="),*end=pid?strchr(pid,']'):NULL;
        if(end) { *pid=0;printf("%s[pid=PID]%s",buf,end+1); }
        else printf("%s",buf);
    }
    fclose(f);puts("END");
}
int main(int argc,char **argv) {
    void *lib;
    logger log;
    char longmsg[1100];
    assert(argc==2);
    mkdir("/tmp/log-compare",0700);
    setenv("TEST_LOG_BASE","/tmp/log-compare",1);
    lib=dlopen(argv[1],RTLD_NOW);if(!lib){fprintf(stderr,"%s\n",dlerror());return 2;}
    log=(logger)dlsym(lib,"asusdebuglog");assert(log);
    printf("custom=%d\n",log(6,"custom",0,0,1,"hello %d\n",42));
    show("custom");
    printf("zero=%d\n",log(0,"zero",0,0,1,"plain\n"));show("zero");
    memset(longmsg,'X',sizeof(longmsg)-1);longmsg[sizeof(longmsg)-1]=0;
    printf("long=%d\n",log(0,"rotate",0,0,1,"%s",longmsg));
    printf("rotate=%d\n",log(0,"rotate",0,0,1,"after\n"));show("rotate");
    unsetenv("TEST_LOG_BASE");
    printf("absolute=%d\n",log(6,"/tmp/log-compare/absolute",0,0,1,"hello\n"));show("absolute");
    printf("timed=%d\n",log(6,"/tmp/log-compare/timed",0,1,1,"hello\n"));show("timed");
    printf("plain=%d\n",log(0,"/tmp/log-compare/plain",0,0,1,"hello\n"));show("plain");
    printf("missing=%d\n",log(6,"/nonexistent-parent-test/file",0,0,1,"hello\n"));
    setenv("TEST_LOG_BASE","/proc/no-such-log-directory",1);
    printf("fallback=%d\n",log(6,"fallback-case",0,0,1,"fallback\n"));
    {
        FILE *f=fopen("/tmp/asusdebuglog/fallback-case","r");
        char line[128];
        printf("FALLBACK: ");
        if(f) { while(fgets(line,sizeof(line),f)) {
            char *pid=strstr(line,"[pid="),*end=pid?strchr(pid,']'):NULL;
            if(end) { *pid=0;printf("%s[pid=PID]%s",line,end+1); }
            else printf("%s",line);
        }fclose(f); }
        else puts("missing");
    }
    unsetenv("TEST_LOG_BASE");
    for (int level=0;level<8;++level) {
        printf("syslog-%d=%d\n",level,log(level,"unused",1,0,0,"event %d\n",level));
        printf("console-%d=%d\n",level,log(level,"unused",2,0,0,"event %d\n",level));
        show("console");
    }
    printf("disabled=%d\n",log(6,"unused",-1,0,0,"ignored\n"));
    return 0;
}
