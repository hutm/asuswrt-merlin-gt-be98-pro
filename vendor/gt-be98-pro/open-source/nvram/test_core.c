/* SPDX-License-Identifier: GPL-2.0-only */
#define _POSIX_C_SOURCE 200809L
#include "core.h"
#include "frame.h"
#include "pair.h"
#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
struct peer {char value[1024];int present,fail,ignore,gets,empty,commits,commit_error;};
static int exchange(void *context,const void *request,size_t size,void *reply,size_t capacity,size_t *returned) {
    struct peer *peer=context;struct nv_frame_view frame;struct nv_pair_view pair;
    unsigned char payload[1024];size_t length=0;unsigned type;
    assert(!nv_frame_decode(request,size,&frame));type=frame.type;
    if(peer->fail) {errno=ETIMEDOUT;return -1;}
    if(type==3) {
        assert(frame.length==4 && !memcmp(frame.payload,"key",4));++peer->gets;
        if(peer->present) assert(!nv_pair_encode(payload,sizeof payload,"key",peer->value,strlen(peer->value)+1,&length));
    } else if(type==2 || type==4) {
        assert(!nv_pair_decode(frame.payload,frame.length,&pair));
        assert(!strcmp(pair.name,"key") && pair.value_len<=sizeof peer->value);
        if(!peer->ignore) {
            peer->present=type==2 && pair.value_len;
            if(peer->present) memcpy(peer->value,pair.value,pair.value_len);
        }
        memcpy(payload,frame.payload,frame.length);length=frame.length;
    } else if(type==7) {
        assert(!frame.length);++peer->commits;
        if(peer->commit_error==1) {errno=ETIMEDOUT;return -1;}
        if(peer->commit_error==2) type=6;
        if(peer->commit_error==3) {payload[0]=0;length=1;}
        if(peer->commit_error==4) {*returned=capacity+1;return 0;}
    } else {
        assert(type==5);type=6;
        if(!peer->empty) {memcpy(payload,"key=value",10);length=10;}
    }
    return nv_frame_encode(reply,capacity,type,0,0,payload,length,returned);
}
static void *stress(void *opaque) {
    struct nv_core *core=opaque;
    for(int i=0;i<100;++i) {
        char *copy=NULL;
        assert(!nv_core_change_ack(core,"key","concurrent",0));
        assert(nv_core_get_copy(core,"key",&copy)==1 && !strcmp(copy,"concurrent"));
        assert(!nv_core_notification(core,NULL,0));
        assert(!strcmp(copy,"concurrent"));free(copy);
    }
    return NULL;
}
int main(void) {
    struct peer peer={0};struct nv_core *core=nv_core_create(exchange,&peer,256*1024);
    char *value=NULL,*retained;unsigned char output[64],message[1280],payload[1024];
    size_t used=0,length,size;pthread_t threads[4];assert(core);
    assert(nv_core_get_copy(core,"key",&value)==0 && !value);
    assert(!nv_core_change_ack(core,"key","",0));
    assert(nv_core_get_copy(core,"key",&value)==1 && !*value);free(value);
    assert(!nv_core_change_ack(core,"key","value",0));
    assert(nv_core_get_copy(core,"key",&retained)==1 && !strcmp(retained,"value"));
    peer.ignore=1;assert(!nv_core_change_ack(core,"key","unapplied",0));
    assert(nv_core_get_copy(core,"key",&value)==1 && !strcmp(value,"value"));free(value);peer.ignore=0;
    peer.fail=1;value=(char *)"unchanged";
    assert(nv_core_get_copy(core,"key",&value)==-1 && errno==ETIMEDOUT && !strcmp(value,"unchanged"));
    assert(nv_core_change_ack(core,"key","unknown",0)==-1 && errno==ETIMEDOUT);peer.fail=0;
    assert(!nv_pair_encode(payload,sizeof payload,"key","unproven",9,&length));
    assert(!nv_frame_encode(message,sizeof message,2,0,0,payload,length,&size));
    assert(nv_core_notification(core,message,size)==1);
    assert(nv_core_get_copy(core,"key",&value)==1 && !strcmp(value,"value"));free(value);
    assert(!nv_core_getall(core,output,sizeof output,&used) && used==10 && !strcmp((char *)output,"key=value"));
    assert(!nv_core_change_ack(core,"key",NULL,1));
    assert(nv_core_get_copy(core,"key",&value)==0 && !value);
    assert(!strcmp(retained,"value"));free(retained);
    {
        FILE *input=tmpfile();size_t confirmed=999;assert(input);
        fputs("key=first\nkey=\nkey=last\n",input);rewind(input);
        assert(!nv_core_load_verified(core,input,&confirmed) && confirmed==3);
        assert(!strcmp(peer.value,"last"));fclose(input);
        peer.ignore=1;input=tmpfile();assert(input);
        fputs("key=unapplied\n",input);rewind(input);
        assert(nv_core_load_verified(core,input,&confirmed)==-1 && errno==EIO && confirmed==0);
        assert(!strcmp(peer.value,"last"));fclose(input);peer.ignore=0;
        input=tmpfile();assert(input);
        fputs("key=partial\n",input);
        for(int i=0;i<1030;++i) fputc(120,input);
        fputc(10,input);rewind(input);
        assert(nv_core_load_verified(core,input,&confirmed)==-1 && errno==EOVERFLOW && confirmed==1);
        assert(!strcmp(peer.value,"partial"));fclose(input);
    }
    {
        char path[]="/tmp/core-save-XXXXXX",contents[64]={0};int directory,fd;
        assert(mkdtemp(path));directory=open(path,O_RDONLY|O_DIRECTORY);assert(directory>=0);
        fd=openat(directory,".kernel_nvram.setting",O_CREAT|O_WRONLY,0600);assert(fd>=0);
        assert(write(fd,"previous\n",9)==9);close(fd);
        peer.fail=1;assert(nv_core_save_at(core,directory,4096)==-1 && errno==ETIMEDOUT);peer.fail=0;
        peer.empty=1;assert(nv_core_save_at(core,directory,4096)==-1 && errno==ENODATA);peer.empty=0;
        assert(faccessat(directory,".kernel_nvram.setting_new",F_OK,0)==-1 && errno==ENOENT);
        fd=openat(directory,".kernel_nvram.setting",O_RDONLY);assert(fd>=0);
        assert(read(fd,contents,sizeof contents)==9 && !strcmp(contents,"previous\n"));close(fd);
        assert(!nv_core_save_at(core,directory,4096));memset(contents,0,sizeof contents);
        fd=openat(directory,".kernel_nvram.setting",O_RDONLY);assert(fd>=0);
        assert(read(fd,contents,sizeof contents)==10 && !strcmp(contents,"key=value\n"));close(fd);
        memset(contents,0,sizeof contents);fd=openat(directory,".kernel_nvram.setting_old",O_RDONLY);assert(fd>=0);
        assert(read(fd,contents,sizeof contents)==9 && !strcmp(contents,"previous\n"));close(fd);
        peer.empty=1;
        assert(nv_core_commit_at(core,directory,4096)==-1 && errno==ENODATA && !peer.commits);
        peer.empty=0;
        fd=openat(directory,".kernel_nvram.setting_new",O_CREAT|O_WRONLY,0600);assert(fd>=0);close(fd);
        assert(nv_core_commit_at(core,directory,4096)==-1 && errno==EEXIST && !peer.commits);
        unlinkat(directory,".kernel_nvram.setting_new",0);
        for(int failure=0;failure<=4;++failure) {
            peer.commit_error=failure;
            assert(nv_core_commit_at(core,directory,4096)==(failure?-3:0));
            if(failure) assert(errno==(failure==1?ETIMEDOUT:EPROTO));
            assert(peer.commits==failure+1);
            memset(contents,0,sizeof contents);
            fd=openat(directory,".kernel_nvram.setting",O_RDONLY);assert(fd>=0);
            assert(read(fd,contents,sizeof contents)==10 && !strcmp(contents,"key=value\n"));close(fd);
        }
        peer.commit_error=0;
        unlinkat(directory,".kernel_nvram.setting",0);unlinkat(directory,".kernel_nvram.setting_old",0);
        close(directory);assert(!rmdir(path));
    }
    for(int i=0;i<4;++i) assert(!pthread_create(&threads[i],NULL,stress,core));
    for(int i=0;i<4;++i) assert(!pthread_join(threads[i],NULL));
    assert(peer.gets>=400);nv_core_destroy(core);
    core=nv_core_create(exchange,&peer,1);assert(core);value=(char *)"unchanged";
    assert(nv_core_get_copy(core,"key",&value)==-1 && errno==ENOSPC && !strcmp(value,"unchanged"));
    nv_core_destroy(core);
    puts("NVRAM core: shared-library operations, verified load, durable save, failure preservation and concurrent calls passed");
    return 0;
}
