#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
struct command {int nargs;const char *name;void *fn;const char *help;};
typedef const struct command *(*lookup_fn)(const char *);
static unsigned count;
static void compare(lookup_fn a,lookup_fn b,const char *name){
 const struct command *x=a(name),*y=b(name);assert((x!=NULL)==(y!=NULL));
 if(x){assert(x->nargs==y->nargs);assert(!strcmp(x->name,y->name));assert(!strcmp(x->help,y->help));assert(x->fn&&y->fn);assert(b(name)==y);}
 count++;
}
int main(int argc,char **argv){
 assert(argc==3);void *a=dlopen(argv[1],RTLD_LAZY|RTLD_LOCAL),*b=dlopen(argv[2],RTLD_LAZY|RTLD_LOCAL);assert(a&&b);
 lookup_fn x=(lookup_fn)dlsym(a,"command_lookup"),y=(lookup_fn)dlsym(b,"command_lookup");assert(x&&y);
 const char *names[]={"media-type","phy-reset","reg","phy-power","vport","stats","ethernet@wirespeed","","unknown"};
 char name[512];
 for(unsigned i=0;i<sizeof(names)/sizeof(*names);i++){
  compare(x,y,names[i]);size_t n=strlen(names[i]);
  for(size_t length=0;length<=n;length++){
   memcpy(name,names[i],length);name[length]=0;compare(x,y,name);
   for(int c=32;c<127;c++){name[length]=c;name[length+1]=0;compare(x,y,name);}
  }
  strcpy(name,names[i]);for(size_t j=0;j<n;j++)name[j]=toupper((unsigned char)name[j]);compare(x,y,name);
 }
 for(size_t length=0;length<sizeof(name);length++){memset(name,'x',length);name[length]=0;compare(x,y,name);}
 assert(y(NULL)==NULL);printf("%u command lookup comparisons; null guard passed\n",count);return 0;
}
