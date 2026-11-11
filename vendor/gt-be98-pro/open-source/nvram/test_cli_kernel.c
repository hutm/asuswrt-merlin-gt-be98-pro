/* SPDX-License-Identifier: GPL-2.0-only */
#define _POSIX_C_SOURCE 200809L
#include "cli_kernel.h"
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
static char events[512];static int used,fail;static const char *stop;
static void event(const char *op,const char *key,const char *value){used+=snprintf(events+used,sizeof events-used,"%s:%s:%s;",op,key,value?value:"<NULL>");assert(used<(int)sizeof events);}
char *nvram_get(const char *key){if(!strcmp(key,"asus_stop_commit"))return (char *)stop;assert(!strcmp(key,"key"));return "value";}
int nvram_set(const char *key,const char *value){event("A",key,value);if(fail){errno=EIO;return -1;}return 3;}
int nvram_unset(const char *key){event("U",key,NULL);return 0;}
int nvram_kset(const char *key,const char *value){event("K",key,value);return 0;}
int wlcsm_nvram_set(const char *key,const char *value){event("W",key,value);return 0;}
static int commit_fail,restart_fail,kcommit_fail;
int nvram_kcommit(void){event("C","",NULL);if(kcommit_fail){errno=EIO;return -1;}return 0;}
int wlcsm_nvram_commit(void){event("WCOMMIT","",NULL);if(commit_fail){errno=EIO;return 1;}return 0;}
int wlcsm_mngr_restart(unsigned radio,unsigned source,int persist,int wait){
 assert(radio==0 && source==1 && wait==0);event("RESTART",persist?"save":"",NULL);
 if(restart_fail){errno=ETIMEDOUT;return 1;}return 0;
}
char *wlcsm_nvram_get(const char *key){(void)key;return NULL;}
char *wlcsm_nvram_get_bitflag(const char *key,unsigned bit){(void)key;(void)bit;return NULL;}
int wlcsm_nvram_set_bitflag(const char *key,unsigned bit,int enabled){(void)key;(void)bit;(void)enabled;return 0;}
static int snapshot_mode;
int nvram_getall(char *out,int capacity){
 const char data[]="key=one\0empty=\0equals=a=b\0\0";
 if(snapshot_mode==1){errno=EIO;return -1;}
 if(snapshot_mode==2){memset(out,'x',(size_t)capacity);return 0;}
 if(snapshot_mode==3){memset(out,'x',(size_t)capacity);out[capacity-1]=0;return 0;}
 if(snapshot_mode==4){const char dump[]="key=one\0restricted=hidden\0\0empty=\0\0";memcpy(out,dump,sizeof dump);return sizeof dump-2;}
 if(snapshot_mode==5)return capacity;
 if(snapshot_mode==6){memset(out,'q',160);memcpy(out+160,"=hidden\0\0",9);return 0;}
 memcpy(out,data,sizeof data);return 0;
}
int invalid_nvram_get_name(char *name){return !strcmp(name,"restricted") || (strlen(name)==160 && strspn(name,"q")==160);}
static void reset(void){used=0;events[0]=0;}
int main(void){
 FILE *out=tmpfile();assert(out);char *set[]={"kset","key=a=b"};assert(nv_cli_kernel(2,set,out)==2&&!strcmp(events,"A:key:a=b;K:key:a=b;"));
 reset();set[1]="key";assert(nv_cli_kernel(2,set,out)==2&&!strcmp(events,"A:key:<NULL>;K:key:<NULL>;"));
 reset();set[0]="kunset";assert(nv_cli_kernel(2,set,out)==2&&!strcmp(events,"U:key:<NULL>;K:key:<NULL>;"));
 reset();set[0]="kset";fail=1;assert(nv_cli_kernel(2,set,out)==-1&&errno==EIO&&!strcmp(events,"A:key:<NULL>;"));fail=0;
 set[0]="kget";assert(nv_cli_kernel(2,set,out)==2);rewind(out);char text[32]={0};assert(fread(text,1,sizeof text,out)==6&&!strcmp(text,"value\n"));
 reset();set[0]="kcommit";stop="0";assert(nv_cli_kernel(1,set,out)==1&&!used);stop=NULL;assert(nv_cli_kernel(1,set,out)==1&&!strcmp(events,"C::<NULL>;"));
 char path[]="/tmp/cli-kernel-XXXXXX";int fd=mkstemp(path);assert(fd>=0);const char data[]="#ignored=x\n key =one\nkey=two\nempty=\n";assert(write(fd,data,sizeof data-1)==sizeof data-1);close(fd);
 reset();set[0]="kernelset";set[1]=path;assert(nv_cli_kernel(2,set,out)==2&&!strcmp(events,"K:key:one;K:key:two;K:empty:;"));reset();set[0]="loadfile";assert(nv_cli_kernel(2,set,out)==2&&!strcmp(events,"W:key:one;W:key:two;W:empty:;"));set[0]="kernelset";unlink(path);
 assert(nv_cli_kernel(2,set,out)==-1&&errno==ENOENT);assert(nv_cli_kernel(1,set,out)==-1&&errno==EINVAL);
 set[0]="savefile";set[1]=path;
 assert(nv_cli_kernel(2,set,out)==2);
 FILE *export=fopen(path,"r");assert(export);char exported[64]={0};
 assert(fread(exported,1,sizeof exported,export)==strlen("key=one\nempty=\nequals=a=b\n"));fclose(export);
 assert(!strcmp(exported,"key=one\nempty=\nequals=a=b\n"));
 for(snapshot_mode=1;snapshot_mode<=3;++snapshot_mode){
  assert(nv_cli_kernel(2,set,out)==-1);assert(errno==(snapshot_mode==1?EIO:EPROTO));
  export=fopen(path,"r");assert(export);char unchanged[64]={0};fread(unchanged,1,sizeof unchanged,export);fclose(export);assert(!strcmp(unchanged,exported));
 }
 snapshot_mode=0;unlink(path);set[1]="/nonexistent/parent/export";assert(nv_cli_kernel(2,set,out)==-1);
 reset();set[0]="restart";assert(nv_cli_kernel(1,set,out)==1&&!strcmp(events,"RESTART::<NULL>;"));
 reset();set[0]="commit";assert(nv_cli_kernel(1,set,out)==1&&!strcmp(events,"WCOMMIT::<NULL>;C::<NULL>;"));
 reset();set[1]="restart";assert(nv_cli_kernel(2,set,out)==2&&!strcmp(events,"RESTART:save:<NULL>;C::<NULL>;"));
 reset();stop="0";assert(nv_cli_kernel(2,set,out)==2&&!used);stop="";assert(nv_cli_kernel(1,set,out)==1&&!used);stop=NULL;
 restart_fail=1;assert(nv_cli_kernel(2,set,out)==-1&&errno==ETIMEDOUT&&!strcmp(events,"RESTART:save:<NULL>;"));restart_fail=0;
 reset();commit_fail=1;assert(nv_cli_kernel(1,set,out)==-1&&errno==EIO&&!strcmp(events,"WCOMMIT::<NULL>;"));commit_fail=0;
 reset();kcommit_fail=1;assert(nv_cli_kernel(1,set,out)==-1&&errno==EIO&&!strcmp(events,"WCOMMIT::<NULL>;C::<NULL>;"));kcommit_fail=0;
 reset();set[1]="get";assert(nv_cli_kernel(2,set,out)==1); /* Preserve following commands. */
 const char *commands[]={"show","dump","getall"};
 for(unsigned i=0;i<3;++i){
  FILE *view=tmpfile();assert(view);set[0]=(char *)commands[i];snapshot_mode=4;
  assert(nv_cli_kernel(1,set,view)==1);rewind(view);char shown[128]={0};fread(shown,1,sizeof shown,view);fclose(view);
  assert(!strcmp(shown,"key=one\nXX ILLEGAL nvram\nempty=\n"));
  for(snapshot_mode=1;snapshot_mode<=5;++snapshot_mode){
   if(snapshot_mode==4)continue;
   view=tmpfile();assert(view);assert(nv_cli_kernel(1,set,view)==-1);assert(ftell(view)==0);fclose(view);
  }
 }
 snapshot_mode=6;set[0]="dump";FILE *view=tmpfile();assert(view);
 assert(nv_cli_kernel(1,set,view)==1 && ftell(view)==0);fclose(view);
 snapshot_mode=0;view=fopen("/dev/full","w");assert(view);assert(!setvbuf(view,NULL,_IONBF,0));
 assert(nv_cli_kernel(1,set,view)==-1 && errno==ENOSPC);fclose(view);
 set[0]="unknown";assert(!nv_cli_kernel(1,set,out));fclose(out);
 puts("Kernel CLI: routing order, NULL values, stop control, file loading and failures passed");return 0;
}
