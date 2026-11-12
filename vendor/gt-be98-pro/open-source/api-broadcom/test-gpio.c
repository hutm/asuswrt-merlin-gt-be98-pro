#include <assert.h>
#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
static unsigned gpio;static int exists_mask,exists_calls,fd_result,read_result,close_calls,export_calls;static char value;
int f_exists(const char *path){char want[64];snprintf(want,sizeof(want),"/sys/class/gpio/gpio%d/value",(int)gpio);assert(!strcmp(path,want));return (exists_mask>>exists_calls++)&1;}
int system(const char *cmd){char want[80];snprintf(want,sizeof(want),"echo %d > /sys/class/gpio/export",(int)gpio);assert(!strcmp(cmd,want));export_calls++;return -1;}
int open64(const char *path,int flags,...){(void)path;assert(flags==O_RDONLY);return fd_result;}
ssize_t read(int fd,void *buf,size_t count){assert(fd==fd_result&&count==1);if(read_result==1)*(char*)buf=value;return read_result;}
int close(int fd){assert(fd==fd_result);close_calls++;return 0;}
int main(int argc,char **argv){assert(argc==2);void *h=dlopen(argv[1],RTLD_LAZY|RTLD_LOCAL);if(!h){fprintf(stderr,"%s",dlerror());return 1;}uint32_t(*fn)(uint32_t)=dlsym(h,"get_gpio");assert(fn);unsigned gpios[]={0,1,31,255,UINT32_MAX};char values[]={'0','1','2','9','x','\0'};unsigned cases=0;
for(unsigned g=0;g<5;g++)for(exists_mask=0;exists_mask<4;exists_mask++)for(fd_result=-1;fd_result<3;fd_result++)for(read_result=-1;read_result<2;read_result++)for(unsigned v=0;v<sizeof(values);v++){
gpio=gpios[g];value=values[v];exists_calls=close_calls=export_calls=0;uint32_t got=fn(gpio),want=UINT32_MAX;
int opened=(exists_mask&2)&&fd_result>0;if(opened&&read_result==1){if(value=='1')want=1;else if(value=='0'||value=='x'||value=='\0')want=0;}
assert(got==want);assert(exists_calls==2);assert(export_calls==!(exists_mask&1));assert(close_calls==!!opened);cases++;
}fprintf(stderr,"%u GPIO candidate cases\n",cases);return 0;}
