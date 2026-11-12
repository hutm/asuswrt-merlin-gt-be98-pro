#include <assert.h>
#include <dlfcn.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
int socket(int a,int b,int c){(void)a;(void)b;(void)c;assert(!"invalid request reached socket");return -1;}
int main(int argc,char**argv){assert(argc==2);void*h=dlopen(argv[1],RTLD_LAZY|RTLD_LOCAL);assert(h);int(*reg[2])(int,unsigned,void*,int)={dlsym(h,"bcm_reg_read_X"),dlsym(h,"bcm_reg_write_X")};int(*mdio[2])(unsigned,void*,int)={dlsym(h,"bcm_pseudo_mdio_read"),dlsym(h,"bcm_pseudo_mdio_write")};unsigned char data[8]={0};int lengths[]={-1,0,9,INT_MAX};unsigned count=0;
for(int k=0;k<4;k++)for(int n=0;n<4;n++)for(int nil=0;nil<2;nil++){if(lengths[n]==0&&!nil)continue;errno=0;void*p=nil?NULL:data;int rc=k<2?reg[k](1,0,p,lengths[n]):mdio[k-2](0,p,lengths[n]);assert(rc==-1&&errno==EINVAL);count++;}
fprintf(stderr,"%u invalid register requests\n",count);return 0;}
