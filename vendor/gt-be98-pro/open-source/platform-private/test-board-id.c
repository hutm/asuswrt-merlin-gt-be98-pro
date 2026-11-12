#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
static const char*text;static int status,reads;
int f_read_string(const char*path,char*buf,int len){assert(!strcmp(path,"/proc/nvram/boardid")&&len==32);reads++;if(status<0)buf[0]=0;else snprintf(buf,(size_t)len,"%s",text);printf("read:%s:%d;",buf,status);return status;}
int main(int argc,char**argv){assert(argc==2);void*h=dlopen(argv[1],RTLD_LAZY|RTLD_LOCAL);if(!h){puts(dlerror());return 1;}int(*fn)(const char*)=dlsym(h,"hnd_boardid_cmp");assert(fn);const char*values[]={NULL,"","GT-BE98_PRO","GT-BE98","96813","boardid\n","1234567890123456789012345678901","12345678901234567890123456789012","aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa"};int statuses[]={-1,0,1,10,31};unsigned cases=0;
for(unsigned a=0;a<9;a++)for(unsigned b=1;b<9;b++)for(unsigned c=0;c<5;c++){text=values[b];status=statuses[c];reads=0;printf("case:%u;",cases++);int rc=fn(values[a]);assert(reads==(values[a]!=NULL));printf("result:%d\n",rc);}
char a[32],b[32];for(unsigned len=1;len<32;len++)for(unsigned pos=0;pos<len;pos++)for(unsigned v=1;v<256;v+=17){memset(a,'a',len);memset(b,'a',len);a[len]=b[len]=0;b[pos]=(char)v;text=b;status=(int)len;reads=0;printf("case:%u;",cases++);int rc=fn(a);assert(reads==1);printf("result:%d\n",rc);}
printf("%u board ID comparisons\n",cases);return 0;}
