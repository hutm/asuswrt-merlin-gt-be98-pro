#include "vlan.h"
#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
typedef wgn_vlan_rule *(*parse_fn)(char *,wgn_vlan_rule *,size_t,size_t *,int);
static unsigned cases;
static int band_result, band_limit=3;
int wgn_get_wl_band_unit(int *unit,int *subunit,char *two,char *five){assert(!unit&&!subunit);printf("band:%s:%s;",two,five);return band_result;}
int wgn_guest_ifcount(int band){assert(band==band_result);printf("limit:%d;",band);return band_limit;}

static void run(parse_fn parse,const char *text,int cfg,size_t capacity,unsigned pattern,int nulls){
 wgn_vlan_rule rules[4];memset(rules,pattern,sizeof(rules));char content[16000],saved[16000];snprintf(content,sizeof(content),"%s",text);strcpy(saved,content);
 size_t count=99;errno=0;wgn_vlan_rule *result=parse(nulls&1?NULL:content,nulls&2?NULL:rules,capacity,nulls&4?NULL:&count,cfg);int error=errno;
 assert(!strcmp(saved,content));printf("%d:%zu:%d:",result==rules?1:result==NULL?0:-1,count,error);
 const unsigned char *bytes=(void *)rules;for(size_t i=0;i<sizeof(rules);i++)if(bytes[i]!=(unsigned char)pattern)printf("%zu=%02x,",i,bytes[i]);putchar('\n');cases++;
}
int main(int argc,char **argv){assert(argc==2);void *h=dlopen(argv[1],RTLD_LAZY|RTLD_LOCAL);if(!h){puts(dlerror());return 1;}parse_fn parse=dlsym(h,"wgn_vlan_list_get_from_content");assert(parse);
 const char *normal[]={"1","20","3","0","f","wl0.1","wl1.1","192.168.1.1/24","1","0","1"};
 const char *texts[]={"", "0", "1", "2", "-1", "junk", "2147483647", "2147483648", "-2147483649", "4294967295", " 1tail", "0x10"};
 for(unsigned f=0;f<11;f++)for(unsigned v=0;v<12;v++)for(int cfg=-1;cfg<4;cfg++)for(size_t cap=0;cap<5;cap++){
  char line[14000]="<";for(unsigned j=0;j<11;j++){if(j)strcat(line,">");strcat(line,j==f?texts[v]:normal[j]);}
  char repeated[16000];snprintf(repeated,sizeof(repeated),"%s%s",line,line);run(parse,repeated,cfg,cap,0xa5,0);
 }
 unsigned lengths[]={0,1,7,8,9,30,31,32,33,1024,1025};
 for(unsigned f=0;f<11;f++)for(unsigned n=0;n<11;n++)for(int cfg=0;cfg<3;cfg++){
  char longfield[1100];memset(longfield,'a',lengths[n]);longfield[lengths[n]]=0;char line[14000]="<";
  for(unsigned j=0;j<11;j++){if(j)strcat(line,">");strcat(line,j==f?longfield:normal[j]);}run(parse,line,cfg,3,0x5a,0);
 }
 const char *bad[]={"","<",">",">>>>>>>>>>>",">>>>>>>>>>>>1", "<bad<1>2>3<>>>>>>>>>>>1", "<a>b>1>d>e>2>f>g>h>3>j>1>extra"};
 for(unsigned i=0;i<7;i++)for(int cfg=-2;cfg<5;cfg++)for(int nulls=0;nulls<8;nulls++)run(parse,bad[i],cfg,4,0xcc,nulls);
 for(band_result=-1;band_result<=4;band_result++)for(band_limit=-1;band_limit<=4;band_limit++)for(unsigned cap=0;cap<=4;cap++){
 run(parse,"<1>20>3>0>f>a>b>sub>1>0>1<1>21>3>0>f>a>b>sub>1>0>1<1>22>3>0>f>a>b>sub>1>0>1",1,cap,0xa5,0);
 }
 fprintf(stderr,"%u VLAN parser comparisons\n",cases);return 0;
}
