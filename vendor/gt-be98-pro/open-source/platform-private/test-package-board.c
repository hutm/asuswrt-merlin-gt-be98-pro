#include <assert.h>
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static const char *identity;static int status;
int f_read_string(const char*path,char*buf,int size){assert(!strcmp(path,"/proc/nvram/boardid")&&size==251);if(status<0)buf[0]=0;else snprintf(buf,(size_t)size,"%s",identity);printf("read:%d:%s;",status,buf);return status;}
int main(int argc,char**argv){assert(argc==2||argc==3);setvbuf(stdout,NULL,_IONBF,0);void*h=dlopen(argv[1],RTLD_LAZY|RTLD_LOCAL);assert(h);int(*fn)(char*)=dlsym(h,"check_pkgtb_boardid");assert(fn);char*image=malloc(16384);assert(image);char longid[280];memset(longid,'a',279);longid[279]=0;const char*ids[]={"GT-BE98_PRO","96813","board\n", "x",longid};unsigned cases=0;
if(argc==3){identity="board";int codes[]={-1,0,1};for(unsigned i=0;i<3;i++){status=codes[i];identity=i==2?"":"board";memset(image,0,16384);memcpy(image,"fdt_board",9);assert(fn(image)==-1);cases++;}identity="board";status=5;for(unsigned i=1;i<9;i++){memset(image,0,16384);memcpy(image+16384-i,"fdt_board",i);assert(fn(image)==-1);cases++;}assert(fn(NULL)==-1);cases++;printf("%u package board boundary cases passed\n",cases);free(image);return 0;}
for(unsigned id=0;id<5;id++)for(unsigned where=0;where<7;where++)for(unsigned mutation=0;mutation<8;mutation++){
 identity=ids[id];status=1;char marker[255]="fdt_";snprintf(marker+4,251,"%s",identity);size_t len=strlen(marker);size_t positions[]={0,1,100,8192,16000,16384-len,16384-len-1};memset(image,0xa5,16384);memcpy(image+positions[where],marker,len);if(mutation<6)image[positions[where]+(mutation%len)]^=1;printf("case:%u;",cases++);int rc=fn(image);assert(rc==(mutation<6?-1:1));printf("result:%d\n",rc);}
printf("%u package board comparisons\n",cases);free(image);return 0;}
