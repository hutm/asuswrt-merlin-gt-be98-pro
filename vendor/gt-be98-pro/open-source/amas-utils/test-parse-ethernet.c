#include <assert.h>
#include <dlfcn.h>
#include <limits.h>
#include <locale.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static int mask, write_mask;
void trans_to_bhmode(int *a,int *b,int *c,int *d,int *e) {
    assert(!*a&&!*b&&!*c&&!*d&&!*e);
    printf("mode:%d:%d;",mask,write_mask);
    if(write_mask)*a=mask;
    *b=123;*c=-2;*d=INT_MIN;*e=INT_MAX;
}
int main(int argc,char **argv) {
    assert(argc==2);assert(setlocale(LC_ALL,"C"));
    void *h=dlopen(argv[1],RTLD_LAZY|RTLD_LOCAL);assert(h);
    int (*parse)(char*)=dlsym(h,"cal_space"),(*enabled)(int)=dlsym(h,"enable_ETH_U");assert(parse&&enabled);
    unsigned cases=0;
    char buf[260];
    for(int c=1;c<256;c++)for(int pos=0;pos<3;pos++) {
        memcpy(buf,"abc",4);buf[pos]=(char)c;printf("parse:%d\n",parse(buf));cases++;
    }
    for(unsigned n=1;n<260;n++)for(unsigned pattern=0;pattern<8;pattern++) {
        for(unsigned i=0;i<n;i++)buf[i]=(i+pattern)%(pattern+1)==0?" \t\n\v\f\r"[(i+pattern)%6]:'x';
        buf[n]=0;printf("parse:%d\n",parse(buf));cases++;
    }
    for(int unit=-1024;unit<=1024;unit++)for(unsigned bit=0;bit<34;bit++)for(write_mask=0;write_mask<2;write_mask++) {
        uint32_t v=bit<32?UINT32_C(1)<<bit:bit==32?0:UINT32_MAX;memcpy(&mask,&v,4);
        printf("eth:%d\n",enabled(unit));cases++;
    }
    uint32_t seed=7;
    for(unsigned i=0;i<10000;i++) {
        seed=seed*1664525u+1013904223u;int unit;memcpy(&unit,&seed,4);
        seed=seed*1664525u+1013904223u;memcpy(&mask,&seed,4);write_mask=1;
        printf("eth:%d\n",enabled(unit));cases++;
    }
    if(getenv("CANDIDATE_DEFENSIVE")){assert(parse(NULL)==0);assert(parse("")==0);}
    fprintf(stderr,"%u parser and Ethernet comparisons\n",cases);
}
