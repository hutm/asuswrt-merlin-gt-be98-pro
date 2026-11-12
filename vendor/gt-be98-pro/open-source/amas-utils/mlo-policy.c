#define _LARGEFILE64_SOURCE
#include <stdio.h>
#include <unistd.h>
extern int nvram_get_int(const char *),nvram_set(const char *,const char *),cprintf(const char *,...);
extern char *nvram_get(const char *);
extern int isMloConnectionMode(void),is_connection_priority_is_wifi_first(void);
static void mark_non_mlo(void){FILE *f=fopen64("/tmp/non_mlo_connection","w");if(f)fclose(f);}
int checkMloWiFi(void){
 int mode=nvram_get_int("amas_ethernet");if(!mode)mode=3;
 if(!isMloConnectionMode()||mode<100||mode>400)return 0;
 nvram_set("wifi_first_detect","1");cprintf("amas_ethernet = %d, change to nonMLO connection Mode\n",mode);return 1;
}
int checkMatchMloCondition(void){
 int match=nvram_get("cap_re_vendor_match")?nvram_get_int("cap_re_vendor_match"):1;
 int skip=nvram_get_int("skip_check_cap_vendor");
 if(!isMloConnectionMode()||match||skip)return 1;
 mark_non_mlo();cprintf("not match CAP vendor, need change to nonMLO connection Mode\n");return 0;
}
int checkMloConnectionChange(void){
 int enabled=nvram_get_int("mld_enable"),ethernet=nvram_get_int("amas_ethernet");if(!ethernet)ethernet=3;
 int mode=isMloConnectionMode(),wifi=is_connection_priority_is_wifi_first();
 if(!enabled)return 0;
 if(mode&&wifi){mark_non_mlo();cprintf("amas_ethernet = %d, change to nonMLO connection Mode\n",ethernet);return 1;}
 if(!mode&&!wifi){unlink("/tmp/non_mlo_connection");cprintf("amas_ethernet = %d, change to MLO connection Mode\n",ethernet);return 1;}
 return 0;
}
