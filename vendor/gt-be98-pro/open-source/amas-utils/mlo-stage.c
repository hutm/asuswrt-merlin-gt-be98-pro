#include <stdio.h>
extern int nvram_get_int(const char *),nvram_set_int(const char *,int),f_exists(const char *);
extern char *nvram_get(const char *);
int isMloConnectionMode(void){
 int enabled=nvram_get_int("mld_enable"),exists=f_exists("/tmp/non_mlo_connection");
 if(!enabled)return 0;
 char *config=nvram_get("wl_mlo_config");return config&&*config&&exists<=0;
}
int is_self_optmz_stage(int band){
 char *present=nvram_get("mlo_skip_re_selfopt");int skip=nvram_get_int("mlo_skip_re_selfopt");
 if(!present&&!skip)skip=1;
 char key[64];snprintf(key,sizeof(key),"amas_wlc%d_optmz",band);
 int mode=isMloConnectionMode();
 if(skip&&mode&&nvram_get_int(key)==1){nvram_set_int(key,0);return 0;}
 if(nvram_get_int(key)==1)return 1;
 return nvram_get_int(key)==2;
}
