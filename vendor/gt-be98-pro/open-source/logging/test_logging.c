#define _FILE_OFFSET_BITS 64
#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
int asusdebuglog(int, char *, int, int, unsigned, const char *, ...);
int rm_asusdebuglog(char *);
char *my_strrchr(const char *, int);
static char *base;
char *nvram_get(const char *name) { return strcmp(name,"asuslog_path") ? NULL : base; }
int main(void) {
    char temp[] = "/tmp/openlog-test-XXXXXX", file[256], text[4096], rotated[256];
    FILE *f;
    struct stat st;
    base = mkdtemp(temp); assert(base);
    assert(asusdebuglog(6,"events",0,0,1,"hello %d\n",42) == 1);
    snprintf(file,sizeof(file),"%s/events",base);
    f=fopen(file,"r"); assert(f); assert(fgets(text,sizeof(text),f)); fclose(f);
    assert(strstr(text,"[info] hello 42\n")); assert(strstr(text,"[pid="));
    memset(text,'X',sizeof(text)-1);text[sizeof(text)-1]=0;
    assert(asusdebuglog(0,"events",0,0,1,"%s",text)==1);
    assert(asusdebuglog(0,"events",0,0,1,"after rotation\n")==1);
    snprintf(rotated,sizeof(rotated),"%s/events-0",base);assert(stat(rotated,&st)==0);
    f=fopen(file,"r"); assert(f); assert(fgets(text,sizeof(text),f)); fclose(f);
    assert(strcmp(text,"after rotation\n")==0);
    assert(st.st_size<1300); /* bounded formatting, including timestamp prefix */
    assert(asusdebuglog(8,"events",0,0,1,"bad")==-1);
    assert(asusdebuglog(6,"never-created",-1,0,1,"ignored")==1);
    assert(my_strrchr("a/b/c",'/')[1]=='c'); assert(my_strrchr("abc",0)==NULL);
    assert(rm_asusdebuglog(file)==0);assert(access(file,F_OK)<0);
    assert(rm_asusdebuglog(file)==0);
    unlink(rotated);rmdir(base);
    puts("logging candidate: formatting, limits, rotation, removal and modes passed");
    return 0;
}
