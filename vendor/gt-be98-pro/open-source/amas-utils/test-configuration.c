#include <assert.h>
#include <dlfcn.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
static int exists,result,present,mode,mapped,states[2],calls;static char *list,*name;
int d_exists(const char *s){printf("exists:%s;",s);return exists;}
int mkdir(const char *s,mode_t m){printf("mkdir:%s:%o;",s,(unsigned)m);return result;}
char *nvram_get(const char *s){printf("get:%s;",s);if(!strcmp(s,"wl_ifnames"))return list;int i=-1;assert(sscanf(s,"wl%d_nband",&i)==1&&i>=0&&i<4);return present&(1<<i)?(char *)"":NULL;}
char *wl_nvname(const char *s,int u,int sub){static char b[32];printf("nvname:%s:%d:%d;",s,u,sub);assert(!strcmp(s,"nband")&&!sub);snprintf(b,sizeof(b),"wl%d_nband",u);return b;}
int nvram_set_int(const char *s,int v){printf("set:%s:%d;",s,v);return result;}
int isMloConnectionMode(void){printf("mode:%d;",mode);return mode;}
char *nvram_pf_get(const char *p,const char *s){printf("pf:%s:%s;",p,s);return name;}
int is_mlo_if(char *s){assert(calls<2);printf("if:%s:%d;",s,states[calls]);return states[calls++];}
int is_mlo_map(char *s){printf("map:%s:%d;",s,mapped);return mapped;}
int main(int argc,char **argv){assert(argc==2);void *h=dlopen(argv[1],RTLD_LAZY|RTLD_LOCAL);assert(h);void (*folder)(void)=dlsym(h,"create_amas_sys_folder"),(*trigger)(void)=dlsym(h,"trigger_opt");int (*wifi)(int)=dlsym(h,"check_wifi_use");assert(folder&&trigger&&wifi);unsigned cases=0;
 int edges[]={INT_MIN,-1,0,1,4,INT_MAX};
 for(unsigned a=0;a<6;a++)for(unsigned b=0;b<6;b++){exists=edges[a];result=edges[b];folder();puts("folder");cases++;}
 char buf[129];for(unsigned len=0;len<129;len++)for(unsigned p=0;p<8;p++)for(present=0;present<16;present++)for(result=-1;result<=0;result++){
  for(unsigned i=0;i<len;i++)buf[i]=(i+p)%(p+1)==0?' ':"ab\t\n"[(i+p)%4];
  buf[len]=0;list=buf;trigger();puts("trigger");cases++;
 }
 list=NULL;trigger();puts("trigger-null");cases++;
 char *names[]={NULL,"","wl0","wl0.1","abcdefghijk"};
 for(unsigned i=0;i<6;i++)for(unsigned m=0;m<6;m++)for(unsigned a=0;a<6;a++)for(unsigned b=0;b<6;b++)for(unsigned r=0;r<6;r++)for(unsigned n=0;n<5;n++){
  mode=edges[m];states[0]=edges[a];states[1]=edges[b];mapped=edges[r];name=names[n];calls=0;printf("wifi:%d\n",wifi(edges[i]));cases++;
 }
 fprintf(stderr,"%u configuration comparisons\n",cases);
}
