/* SPDX-License-Identifier: GPL-2.0-only */
#include <stdio.h>
#include <string.h>
#include <time.h>
extern char *get_auth_code(char *,char *,int);
extern char *hw_auth_check(char *,char *,time_t,char *,int);
int auth_validate(char *id,char *key){
 if(!id||!key)return 0;
 time_t timestamp=time(NULL);
 char input[128]={0},local[65]={0},peer[65]={0};
 snprintf(input,sizeof(input)-1,"%ld|%s",(long)timestamp,key);
 char *code=get_auth_code(input,local,sizeof(local));if(!code)return 0;
 char *reply=hw_auth_check(id,code,timestamp,peer,sizeof(peer));if(!reply)return 0;
 snprintf(input,sizeof(input)-1,"%ld|%s|%s",(long)timestamp,key,id);
 code=get_auth_code(input,local,sizeof(local));return code&&!strcmp(reply,code);
}
