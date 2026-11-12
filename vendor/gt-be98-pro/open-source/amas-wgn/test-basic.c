#include <stdint.h>
#include <stdio.h>
#include <assert.h>
#include <dlfcn.h>
int main(int argc,char **argv){assert(argc==2);void *h=dlopen(argv[1],RTLD_LAZY|RTLD_LOCAL);assert(h);int (*mask)(uint32_t)=dlsym(h,"ipmask_to_int");int (*compare)(const void *,const void *)=dlsym(h,"compare");assert(mask&&compare);unsigned cases=0;
 for(uint32_t bits=0;bits<65536;bits++){
  uint32_t values[]={bits<<16,(bits<<16)|65535,bits,0xffff0000|bits};
  for(unsigned i=0;i<4;i++){printf("mask:%08x:%d\n",values[i],mask(values[i]));cases++;}
 }
 uint32_t values[]={0,1,0x7fffffff,0x80000000,0xfffffffe,0xffffffff};
 for(unsigned a=0;a<6;a++)for(unsigned b=0;b<6;b++){printf("compare:%d\n",compare(&values[a],&values[b]));cases++;}
 uint32_t state=123456789;
 for(unsigned i=0;i<50000;i++){state^=state<<13;state^=state>>17;state^=state<<5;uint32_t x=state,y=state*1664525u+1013904223u;printf("random:%d:%d\n",mask(x),compare(&x,&y));cases+=2;}
 fprintf(stderr,"%u subnet-helper comparisons\n",cases);return 0;
}
