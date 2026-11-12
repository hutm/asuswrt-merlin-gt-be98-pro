#include <assert.h>
#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
static int model,calls;
void dbg(const char*fmt,...){va_list ap;va_start(ap,fmt);vprintf(fmt,ap);va_end(ap);}
int cprintf(const char*fmt,...){va_list ap;va_start(ap,fmt);int n=vprintf(fmt,ap);va_end(ap);return n;}
int get_model(void){calls++;printf("model:%d;",model);return model;}
int main(int argc,char**argv){assert(argc==2||argc==3);void*h=dlopen(argv[1],RTLD_LAZY|RTLD_LOCAL);if(!h){puts(dlerror());return 1;}void(*dump)(const void*)=dlsym(h,"dumpWfiTag");int(*check)(char*)=dlsym(h,"check_imagefile");int(*fw)(void)=dlsym(h,"fw_check");assert(dump&&check&&fw);
if(argc==3){dump(NULL);assert(check(NULL)==0);puts("2 null diagnostics cases passed");return 0;}
uint32_t words[5],state=1;unsigned cases=0;
for(unsigned seed=0;seed<1024;seed++){for(unsigned j=0;j<5;j++){state=state*1664525u+1013904223u;words[j]=state;}printf("case:%u;",cases++);dump(words);}
char *names[]={"","/tmp/linux.trx","image with spaces.pkgtb","%s %n\n","missing"};
for(model=-1;model<256;model++)for(unsigned n=0;n<5;n++){calls=0;printf("case:%u;",cases++);assert(check(names[n])==0&&calls==1);assert(fw()==0&&calls==1);}
printf("%u image diagnostics scenarios\n",cases);return 0;}
