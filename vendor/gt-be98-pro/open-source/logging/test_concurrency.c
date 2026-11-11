#define _FILE_OFFSET_BITS 64
#define _POSIX_C_SOURCE 200809L
#include <assert.h>
#include <errno.h>
#include <dirent.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
int asusdebuglog(int, char *, int, int, unsigned, const char *, ...);
static char *base;
char *nvram_get(const char *name) { return strcmp(name,"asuslog_path") ? NULL : base; }
static void *writer(void *arg) {
    long id = (long)arg;
    int i;
    for (i=0;i<100;++i)
        assert(asusdebuglog(6,"concurrent",0,0,1,"@%ld:%d message-end\n",id,i)==1);
    return NULL;
}
int main(void) {
    char temp[]="/tmp/log-concurrency-XXXXXX",path[512],line[2048];
    unsigned seen[8][100]={{0}},count=0;
    pid_t children[4];
    pthread_t threads[4];
    DIR *dir;
    struct dirent *entry;
    int i,status;
    base=mkdtemp(temp); assert(base);
    for(i=0;i<4;++i) {
        children[i]=fork();assert(children[i]>=0);
        if(children[i]==0) { writer((void *)(long)i);_exit(0); }
    }
    for(i=0;i<4;++i) {
        assert(waitpid(children[i],&status,0)==children[i]);
        assert(WIFEXITED(status)&&WEXITSTATUS(status)==0);
    }
    for(i=0;i<4;++i) assert(pthread_create(&threads[i],NULL,writer,(void *)(long)(i+4))==0);
    for(i=0;i<4;++i) assert(pthread_join(threads[i],NULL)==0);
    dir=opendir(base); assert(dir);
    while((errno=0, entry=readdir(dir))) {
        FILE *f;
        if(entry->d_name[0]=='.')continue;
        snprintf(path,sizeof(path),"%s/%s",base,entry->d_name);
        f=fopen(path,"r"); assert(f);
        while(fgets(line,sizeof(line),f)) {
            unsigned id,n;
            char *start=strchr(line,'@');
            assert(start && sscanf(start,"@%u:%u",&id,&n)==2);
            assert(id<8 && n<100 && strstr(start," message-end\n"));
            assert(seen[id][n]++==0);++count;
        }
        assert(!ferror(f));fclose(f);unlink(path);
    }
    assert(errno==0);
    closedir(dir);rmdir(base);
    assert(count==800);
    puts("logging concurrency: 800 unique complete records retained across rotation");
    return 0;
}
