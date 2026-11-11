/* SPDX-License-Identifier: GPL-2.0-only */
#define _POSIX_C_SOURCE 200809L
#include "cli_kernel.h"
#include "runtime.h"
#include "settings.h"
#include "manufacturing.h"
#include <stdlib.h>
static int apply(void *unused,const char *key,const char *value) {
    return unused?wlcsm_nvram_set(key,value):nvram_kset(key,value);
}
/* Validate the entire bounded snapshot before opening/truncating the export. */
static int savefile(const char *path,FILE *output) {
    const size_t capacity=262144;size_t pos=0,end;char *data=malloc(capacity);
    FILE *file;int result=0,saved;
    if(!data) return -1;
    memset(data,0xff,capacity);
    if(nvram_getall(data,(int)capacity)<0) {saved=errno;free(data);errno=saved;return -1;}
    while(pos<capacity && data[pos]) {
        char *nul=memchr(data+pos,0,capacity-pos);
        if(!nul) break;
        pos=(size_t)(nul-data)+1;
    }
    if(pos>=capacity || data[pos]) {free(data);errno=EPROTO;return -1;}
    end=pos;
    file=fopen(path,"w");
    if(!file) {saved=errno;free(data);errno=saved;return -1;}
    for(pos=0;pos<end;pos+=strlen(data+pos)+1) {
        if(fputs(data+pos,file)==EOF || fputc('\n',file)==EOF) {result=-1;break;}
    }
    saved=errno;
    if(fclose(file) && !result) {result=-1;saved=errno;}
    free(data);errno=saved;
    if(!result && fprintf(output,"save nvram to %s done!\n",path)<0) return -1;
    return result;
}
int nv_cli_kernel(int argc,char *const argv[],FILE *output) {
    const char *command;char *copy,*value;int result,saved;FILE *input;
    if(argc<=0 || !argv || !argv[0] || !output) {errno=EINVAL;return -1;}
    result=nv_cli_flags(argc,argv,output);if(result) return result;
    command=argv[0];
    if(!strcmp(command,"savefile")) {
        const char *path=argc>1 && argv[1]?argv[1]:"/data/nvramdefault.txt";
        return savefile(path,output)?-1:(argc>1?2:1);
    }
    if(!strcmp(command,"loadfile")) {
        const char *path=argc>1 && argv[1]?argv[1]:"/data/nvramdefault.txt";
        input=fopen(path,"r");if(!input) return -1;
        result=nv_settings_load(input,apply,&result);saved=errno;
        if(fclose(input) && !result) return -1;
        errno=saved;return result?-1:(argc>1?2:1);
    }
    if(!strcmp(command,"restore_mfg")) {
        if(argc<2 || !argv[1]) {errno=EINVAL;return -1;}
        return nv_mfg_restore("/mnt/nvram/nvram.nvm",argv[1],"/proc/nvram/BaseMacAddr")?-1:2;
    }
    if(!strcmp(command,"restart"))
        return wlcsm_mngr_restart(0,1,0,0)?-1:1;
    if(!strcmp(command,"commit")) {
        int restart=argc>1 && argv[1] && !strcmp(argv[1],"restart");
        int consumed=restart?2:1;
        if(nvram_get("asus_stop_commit")) return consumed;
        result=restart?wlcsm_mngr_restart(0,1,1,0):wlcsm_nvram_commit();
        if(result) return -1;
        return nvram_kcommit()?-1:consumed;
    }
    if(!strcmp(command,"kcommit")) {
        if(nvram_get("asus_stop_commit")) return 1;
        return nvram_kcommit()?-1:1;
    }
    if(strcmp(command,"kget") && strcmp(command,"kset") && strcmp(command,"kunset") && strcmp(command,"kernelset")) return 0;
    if(argc<2 || !argv[1]) {errno=EINVAL;return -1;}
    if(!strcmp(command,"kget")) {
        value=nvram_get(argv[1]);
        if(value && (fputs(value,output)==EOF || fputc('\n',output)==EOF)) return -1;
        return 2;
    }
    if(!strcmp(command,"kunset")) {
        if(nvram_unset(argv[1])<0 || nvram_kset(argv[1],NULL)) return -1;
        return 2;
    }
    if(!strcmp(command,"kset")) {
        copy=strdup(argv[1]);if(!copy) return -1;
        value=strchr(copy,'=');if(value) *value++=0;
        result=nvram_set(copy,value);
        if(result>=0) result=nvram_kset(copy,value);
        saved=errno;free(copy);errno=saved;return result<0?-1:2;
    }
    input=fopen(argv[1],"r");if(!input) return -1;
    result=nv_settings_load(input,apply,NULL);saved=errno;
    if(fclose(input) && !result) return -1;
    errno=saved;return result?-1:2;
}
