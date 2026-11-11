/* SPDX-License-Identifier: GPL-2.0-only */
#define _GNU_SOURCE
#include <dlfcn.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int invalid_program_check(void) { return 0; }
int psname(void) { return 0; }
int logmessage_normal(void) { return 0; }
int jffs_nvram_get(void) { return 0; }
int large_nvram(void) { return 0; }
int jffs_nvram_unset(void) { return 0; }
int invalid_nvram_get_program(void) { return 0; }
int jffs_nvram_set(void) { return 0; }
int jffs_nvram_getall(void) { return 0; }

struct tuple {char *name,*value;unsigned short len,type,access,enc;struct tuple *next;};
struct tuple router_defaults[]={{"test_exact","",0,0,0,1,0},{0}};
struct tuple router_prefix_defaults[]={{"wl","",0,0,0,0,0},{"lan","",0,0,0,0,0},{"wgc","",0,0,0,0,0},{0}};
int invalid_nvram_get_name(char *name) { return !strcmp(name,"restricted"); }
int dec_nvram(char *name,char *input,char *output) {
 if(strcmp(name,"test_exact") || strcmp(input,"ciphertext")) abort();
 const char *fault=getenv("SAVE_FAULT");
 if(fault && !strcmp(fault,"decryptfail")){errno=EIO;return -1;}
 if(fault && !strcmp(fault,"decryptunterminated")){memset(output,120,4096);return 0;}
 if(fault && !strcmp(fault,"expansion")){memset(output,120,4095);output[4095]=0;return 0;}
 strcpy(output,"decrypted");return 0;
}
void sync(void) {if(getenv("ERASE_FIXTURE")) puts("SYNC");}
int system(const char *command) {if(!getenv("ERASE_FIXTURE")) abort();printf("SYSTEM[%s]\n",command);return getenv("ERASE_FAILURE")?256:0;}
int unlink(const char *path) {if(!getenv("ERASE_FIXTURE")) abort();printf("UNLINK[%s]\n",path);
 const char *fault=getenv("UNLINK_FAULT");
 if(fault){errno=!strcmp(fault,"missing")?ENOENT:EACCES;return -1;}return 0;}
static void print_escaped(const char *s) {
    if(!s) { printf("NULL");return; }
    for(;*s;++s) {
        if(*s=='\n') printf("\\n");
        else if(*s=='\r') printf("\\r");
        else if(*s=='\t') printf("\\t");
        else putchar(*s);
    }
}
int nvram_kset(const char *name,const char *value) {
    printf("SET[");print_escaped(name);printf("]=[");print_escaped(value);puts("]");return 0;
}
int wlcsm_nvram_set(const char *name,const char *value) { puts("WSET");return nvram_kset(name,value); }
int nvram_set(const char *name,const char *value) { puts("ASET");return nvram_kset(name,value); }

FILE *fopen(const char *path,const char *mode) {
    FILE *(*real)(const char *,const char *)=dlsym(RTLD_NEXT,"fopen64");
    FILE *result=real(path,mode);
    if(strcmp(path,"/dev/urandom")) fprintf(stderr,"fopen %s %s: %s\n",path,mode,result?"ok":strerror(errno));
    return result;
}

char *nvram_get(const char *name) {printf("GET[%s]\n",name);return NULL;}
char *wlcsm_nvram_get(const char *name) {printf("WGET[%s]\n",name);return "synthetic";}
int nvram_getall(char *out,int size) {const char data[]="key=one\0empty=\0equals=a=b\0\0";if(size<(int)sizeof data)abort();memset(out,0,size);memcpy(out,data,sizeof data);
 const char *fault=getenv("SAVE_FAULT");
 if(fault && !strcmp(fault,"readfail")){errno=EIO;return -1;}
 if(fault && !strcmp(fault,"unterminated")){memset(out,120,size);return 0;}
 if(fault && !strcmp(fault,"oversized"))return size;
 if(fault && !strcmp(fault,"expansion")){
  const char item[]="test_exact=ciphertext";char *next=out;
  for(int i=0;i<70;++i){memcpy(next,item,sizeof item);next+=sizeof item;}
  *next=0;return 0;
 }
 if(getenv("SAVE_FIXTURE")) {
  const char save[]="lan_ipaddr=192.0.2.1\0test_exact=ciphertext\0empty=\0equals=a=b\0\0";
  memcpy(out,save,sizeof save);return 0;
 }
 if(getenv("DUMP_FIXTURE")) {
  const char dump[]="key=one\0restricted=hidden\0\0empty=\0equals=a=b\0\0";
  memcpy(out,dump,sizeof dump);return sizeof dump-2;
 }
 return 0;}
int wlcsm_nvram_getall(char *out,int size) {return nvram_getall(out,size);}
int nvram_commit(void) {puts("COMMIT");return 0;}
int nvram_kcommit(void) {puts("KCOMMIT");return 0;}

int nvram_unset(const char *name){printf("AUNSET[%s]\n",name);return 0;}
int wlcsm_nvram_unset(const char *name){printf("WUNSET[%s]\n",name);return 0;}

int wlcsm_nvram_commit(void) {puts("WCOMMIT");return 0;}
int wlcsm_mngr_restart(unsigned radio,unsigned source,int persist,int wait) {
 printf("RESTART[%u,%u,%d,%d]\n",radio,source,persist,wait);return 0;
}
