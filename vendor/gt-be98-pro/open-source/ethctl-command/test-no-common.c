#define main unused_fixture_main
#include "test-reset.c"
#undef main
int main(int argc,char **argv){
 assert(argc==2);void *h=dlopen(argv[1],RTLD_LAZY|RTLD_LOCAL);assert(h);const struct command *(*lookup)(const char *)=dlsym(h,"command_lookup");assert(lookup);callback fn=lookup("phy-reset")->fn;assert(fn);
 char *args[]={"ethctl","eth0","phy-reset",NULL,NULL};struct ifreq ifr;memset(&ifr,0,sizeof(ifr));strcpy(ifr.ifr_name,"eth0");map=0;phy=0;scenario=100;failstep=0;calls=0;
 assert(fn(23,&ifr,3,args)==0);assert(calls==11);return 0;
}
