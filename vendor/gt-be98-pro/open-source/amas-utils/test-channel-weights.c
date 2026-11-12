#include <assert.h>
#include <dlfcn.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
static char *disabled,*weights,*weight;static int counts[2],calls,result,mutate;
char *nvram_get(const char *s){printf("get:%s;",s);if(!strcmp(s,"disable_amas_chwt"))return disabled;if(!strcmp(s,"wl0_acs_channel_weights"))return weights;assert(!strcmp(s,"acs_chwt"));return weight;}
int num_of_5g_if(void){assert(calls<2);printf("count:%d;",counts[calls]);return counts[calls++];}
int nvram_set(const char *s,const char *v){printf("set:%s:%s;",s,v);return result;}
int cprintf(const char *fmt,...){va_list ap;va_start(ap,fmt);int r=vprintf(fmt,ap);va_end(ap);return r;}
int _add_to_list(const char *item,char *list,int size,char sep){assert(size==512&&sep==',');printf("add:%s:%s:%d:%c;",item,list,size,sep);if(mutate){size_t n=strlen(list);if(n<510){list[n++]=sep;list[n]=0;snprintf(list+n,(size_t)size-n,"%s",item);}}return result;}
int main(int argc,char **argv){assert(argc==2);void *h=dlopen(argv[1],RTLD_LAZY|RTLD_LOCAL);assert(h);int (*fn)(int)=dlsym(h,"acs_set_chwt");assert(fn);unsigned cases=0;
 char *strings[]={NULL,"","0","1","01","85","100,85","123456789abcdef"};int values[]={INT_MIN,-1,0,1,2,3,INT_MAX};
 for(int ch=-5;ch<180;ch++)for(unsigned d=0;d<8;d++)for(unsigned a=0;a<7;a++)for(unsigned b=0;b<7;b++){disabled=strings[d];counts[0]=values[a];counts[1]=values[b];weights=strings[(a+b)%8];weight=strings[(a+2*b)%8];calls=0;result=values[(a+b)%7];mutate=(a+b)%2;printf("ret:%d\n",fn(ch));cases++;}
 char buf[601];disabled=NULL;counts[0]=counts[1]=1;
 for(unsigned n=0;n<601;n++)for(unsigned w=0;w<8;w++)for(mutate=0;mutate<2;mutate++){memset(buf,'x',n);buf[n]=0;weights=buf;weight=strings[w];calls=0;printf("ret:%d\n",fn(100));cases++;}
 fprintf(stderr,"%u channel weight comparisons\n",cases);
}
