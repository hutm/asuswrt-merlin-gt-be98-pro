#include <assert.h>
#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
static const char *flags,*territory,*location,*coordinate,*odm;
static int mode,model;static unsigned count;
char *nvram_get(const char *key){printf("get:%s;",key);if(!strcmp(key,"asusctrl_flags"))return (char *)flags;if(!strcmp(key,"territory_code"))return (char *)territory;if(!strcmp(key,"location_code"))return (char *)location;if(!strcmp(key,"coordinate"))return (char *)coordinate;if(!strcmp(key,"odmpid"))return (char *)odm;abort();}
int nvram_get_int(const char *key){assert(!strcmp(key,"SG_mode"));printf("int:%s;",key);return mode;}
int nvram_get_hex(const char *key){assert(!strcmp(key,"asusctrl_flags"));printf("hex:%s;",key);long long n=strtoll(flags?flags:"",NULL,16);if(n>UINT32_MAX||n<-(long long)UINT32_MAX){errno=ERANGE;return -1;}return (int)(uint32_t)n;}
int nvram_set_int(const char *key,int n){printf("setint:%s:%d;",key,n);return 0;}
int nvram_set(const char *key,const char *v){printf("set:%s:%s;",key,v);return 0;}
int nvram_commit(void){printf("commit;");return 0;}
int notify_rc(const char *s){printf("notify:%s;",s);return 0;}
int get_model(void){printf("model;");return model;}
struct defaults_entry {int model;char *odmpid,*territory,*name,*value;};
int main(int argc,char **argv){assert(argc==2);void *h=dlopen(argv[1],RTLD_LAZY|RTLD_LOCAL);if(!h){puts(dlerror());return 1;}
 int (*en)(int)=dlsym(h,"asus_ctrl_en");int (*ignore)(void)=dlsym(h,"asus_ctrl_ignore");int (*sg)(void)=dlsym(h,"setting_SG_mode_wps");int (*ctrl)(char *,int)=dlsym(h,"asus_ctrl_nv");int (*restore)(void)=dlsym(h,"asus_ctrl_nv_restore");
 const char *values[]={NULL,"","0","40","7f","-1","80000000","ffffffff","100000000"," 0x40","-80000001","junk","40tail"};
 for(unsigned a=0;a<sizeof(values)/sizeof(*values);a++){
  flags=values[a];for(int shift=-260;shift<=260;shift++){int n=en(shift);printf("en:%d:%d\n",shift,n);count++;}
  int n=ignore();printf("ignore:%d\n",n);count++;
  for(mode=-1;mode<3;mode++){
   errno=0;n=sg();printf("sg:%d:%d\n",n,errno);count++;
   errno=0;n=restore();printf("restore:%d:%d\n",n,errno);count++;
   for(unsigned b=1;b<sizeof(values)/sizeof(*values);b++)for(int notify=-1;notify<2;notify++){
    errno=0;n=ctrl((char *)values[b],notify);printf("ctrl:%d:%d\n",n,errno);count++;
   }
  }
 }
 const char *codes[]={NULL,"","C","CN","CT","CN/01","cn","EU","UK","IL","US","CNCNCNCNCNCNCNCNCN"};
 const char *coords[]={NULL,"","1","1,2","1,2,CN","1,2,cn","1,2,US","1,2,CN,extra",",,CN","1,2, CN","1,2,CT","1,2,CNxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"};
 int (*cn)(void)=dlsym(h,"is_CN_sku"),(*loc)(void)=dlsym(h,"is_CN_location"),(*eu)(void)=dlsym(h,"is_EU_sku"),(*ddns)(void)=dlsym(h,"noasusddns");
 for(unsigned a=0;a<sizeof(codes)/sizeof(*codes);a++)for(unsigned b=0;b<sizeof(coords)/sizeof(*coords);b++){
  territory=location=codes[a];coordinate=coords[b];int n=cn();printf("cn:%d\n",n);n=loc();printf("loc:%d\n",n);n=eu();printf("eu:%d\n",n);n=ddns();printf("ddns:%d\n",n);count+=4;
 }
 struct defaults_entry *table=dlsym(h,"tcode_init_nvram_list");char *(*def)(const char *)=dlsym(h,"tcode_default_get");
 for(unsigned row=0;table[row].model;row++)for(unsigned region=0;table[region].model;region++)for(int m=0;m<3;m++){
  model=m==0?-1:(m==1?168:169);odm="";territory=table[region].territory;
  char *v=def(table[row].name);printf("actual:%u:%u:%d:%s\n",row,region,model,v?v:"<NULL>");count++;
 }
 char longcode[100];for(int len=0;len<100;len++){
  memset(longcode,'x',sizeof(longcode));if(len>=2){longcode[0]='C';longcode[1]='N';}longcode[len]=0;
  territory=location=coordinate=longcode;int n=cn();printf("longcn:%d\n",n);n=loc();printf("longloc:%d\n",n);n=eu();printf("longeu:%d\n",n);n=ddns();printf("longddns:%d\n",n);count+=4;
 }
 const char *names[]={"foo","bar",""};const char *odms[]={NULL,"","model"};const char *regions[]={"","CN","CN/01","US","1234567"};
 for(int rowmodel=-1;rowmodel<3;rowmodel++)for(model=-1;model<4;model++)for(unsigned a=0;a<3;a++)for(unsigned b=0;b<5;b++)for(unsigned c=0;c<5;c++)for(unsigned n=0;n<3;n++){
  memset(table,0,2*sizeof(*table));table[0]=(struct defaults_entry){rowmodel,(char *)odms[a],(char *)regions[b],"foo","value"};
  territory=regions[c];odm="model";char *v=def(names[n]);printf("default:%s\n",v?v:"<NULL>");count++;
 }
 fprintf(stderr,"%u region/settings comparison cases\n",count);return 0;
}
