#include <assert.h>
#include <dlfcn.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
static int count,bands[8],mode,mapped,integer;static char *ifname;static double real;
int num_of_wl_if(void){printf("count:%d;",count);return count;}
int nvram_get_int(const char *key){printf("get:%s;",key);for(int i=0;i<8;i++){char k[32];snprintf(k,sizeof(k),"wl%d_nband",i);if(!strcmp(k,key))return bands[i];}assert(0);return 0;}
int isMloConnectionMode(void){printf("mode:%d;",mode);return mode;}
char *nvram_pf_get(const char *p,const char *s){printf("pf:%s:%s;",p,s);return ifname;}
int is_mlo_map(char *s){printf("map:%s;",s);return mapped;}
int get_wifi_2G_maxpower(void){printf("2G;");return integer;}
int get_wifi_6GH_maxpower(void){printf("6GH;");return integer;}
double get_wifi_5G_maxpower(void){printf("5G;");return real;}
double get_wifi_5GH_maxpower(void){printf("5GH;");return real;}
double get_wifi_6G_maxpower(void){printf("6G;");return real;}
int main(int argc,char **argv){assert(argc==2);void *h=dlopen(argv[1],RTLD_LAZY|RTLD_LOCAL);assert(h);
 int (*fh)(void)=dlsym(h,"get_mlo_fh_band"),(*map)(int)=dlsym(h,"is_mlo_map_by_bandindex");double (*power)(int)=dlsym(h,"get_wifi_tx_maxpower");assert(fh&&map&&power);
 unsigned cases=0;int vals[]={0,1,2,4,-1};
 for(count=-1;count<=8;count++){unsigned patterns=1;for(int i=0;i<count;i++)patterns*=5;for(unsigned p=0;p<patterns;p++){unsigned v=p;for(int i=0;i<count;i++){bands[i]=vals[v%5];v/=5;}printf("fh:%d\n",fh());cases++;}}
 int edges[]={INT_MIN,-1,0,1,4,100,INT_MAX};char *names[]={NULL,"","wl0","wl0.1","abcdefg","abcdefghijk","wl0 wl1"};
 for(unsigned i=0;i<7;i++)for(unsigned m=0;m<7;m++)for(unsigned r=0;r<7;r++)for(unsigned n=0;n<7;n++){mode=edges[m];mapped=edges[r];ifname=names[n];printf("map:%d\n",map(edges[i]));cases++;}
 uint32_t seed=9;for(unsigned p=0;p<1000;p++){seed=seed*1664525u+1013904223u;memcpy(&integer,&seed,4);real=(double)integer/8.0;for(int b=-1;b<70;b++){double v=power(b);uint64_t bits;memcpy(&bits,&v,8);printf("power:%016llx\n",(unsigned long long)bits);cases++;}}
 fprintf(stderr,"%u radio policy comparisons\n",cases);
}
