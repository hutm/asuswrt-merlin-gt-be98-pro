#define _GNU_SOURCE
#include <assert.h>
#include <dlfcn.h>
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
static const char *contents;
FILE *fopen64(const char *path,const char *mode){assert(!strcmp(path,"/proc/net/vlan/config")&&!strcmp(mode,"r"));printf("open;");if(!contents){errno=ENOENT;return NULL;}FILE *f=*contents?fmemopen((void *)contents,strlen(contents),"r"):tmpfile();assert(f);errno=0;return f;}
int cprintf(const char *format,...){va_list ap;va_start(ap,format);int n=vprintf(format,ap);va_end(ap);return n;}
int main(int argc,char **argv){assert(argc==2);void *h=dlopen(argv[1],RTLD_LAZY|RTLD_LOCAL);assert(h);int (*check)(char *,char *)=dlsym(h,"wgn_check_vlan_invalid");assert(check);unsigned cases=0;
 const char *files[]={NULL,"","header\n","header\nheader\n","h\nh\nvlan20 | 20 | eth0\n","h\nh\nbad\nvlan20 | 20 | eth0\nvlan30 | 30 | eth1\n","h\nh\nvlan20 | 20 | eth0\nvlan20 | 21 | eth2\n","vlan20 | 20 | eth9\nvlan20 | 20 | eth8\nvlan20 | 20 | eth7\n","h\nh\n  vlan20   |  20 | eth0  extra\n","h\nh\nvlan20 x y z\n","h\nh\nvlan20 a b c eth5"};
 char *words[]={"","vlan","vlan20","vlan30","eth0","vlan20x","not found"};
 for(unsigned f=0;f<11;f++)for(unsigned w=0;w<7;w++){
 contents=files[f];unsigned char out[32];memset(out,0xa5,sizeof(out));errno=0;int result=check(words[w],(char *)out);int error=errno;printf("result:%d:%d:",result,error);for(unsigned i=0;i<32;i++)printf("%02x",out[i]);putchar('\n');cases++;
 }
 fprintf(stderr,"%u VLAN validation comparisons\n",cases);
}
