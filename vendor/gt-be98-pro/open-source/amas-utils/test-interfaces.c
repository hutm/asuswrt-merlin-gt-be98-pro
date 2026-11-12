#include <assert.h>
#include <dlfcn.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
static char *values[5], *mssid, *list;
static int band, subunit, mode, kind;
char *nvram_get(const char *key) {
    printf("get:%s;", key);
    if (kind == 1) return mssid;
    for (int i=0;i<5;i++) { char k[16];snprintf(k,sizeof(k),"wl%d_ifname",i);if(!strcmp(k,key))return values[i]; }
    abort();
}
int nvram_get_int(const char *key) {
    printf("int:%s;",key);
    if(!strcmp(key,"dwb_band"))return band;
    assert(!strcmp(key,"mlo_dwb_mssid_subunit"));return subunit;
}
static char *provider(char *out,size_t n) {
    assert(n==32); for(size_t i=0;i<n;i++)assert(out[i]==0);
    if(list)snprintf(out,n,"%s",list);
    return mode==0?NULL:mode==1?out:(char *)"alternate-return";
}
char *get_compatible_network(int unit,char *out,int n) {
    assert(unit==-1);printf("compatible:%d:%d;",unit,n);return provider(out,(size_t)n);
}
char *get_fh_if_prefix(char *out,size_t n) {
    printf("fh:%zu;",n);return provider(out,n);
}
int main(int argc,char **argv) {
    assert(argc==2);void *h=dlopen(argv[1],RTLD_LAZY|RTLD_LOCAL);if(!h){fprintf(stderr,"%s",dlerror());return 1;}
    int (*wl)(char*)=dlsym(h,"wl_unit"),(*dw)(char*)=dlsym(h,"is_mlo_dwb_mssid"),(*co)(char*)=dlsym(h,"is_compatible_network"),(*fh)(char*)=dlsym(h,"is_mainFH_network");assert(wl&&dw&&co&&fh);
    char *strings[]={NULL,"","wl","wl0","wl0.1","eth0","eth1","wl4","wl0 wl1"," wl0  wl1 ","wl0\twl1","wl0\nwl1","abcdefghijklmnopqrstuvwxyz123456789","a"};
    unsigned cases=0;
    for(unsigned pattern=0;pattern<16807;pattern++) {
        unsigned p=pattern;for(int i=0;i<5;i++){values[i]=strings[p%7];p/=7;}
        for(unsigned q=1;q<sizeof(strings)/sizeof(*strings);q++){printf("wl:%d\n",wl(strings[q]));cases++;}
    }
    kind=1;int ints[]={INT_MIN,-100,-1,0,1,4,10,100,INT_MAX};
    for(unsigned b=0;b<9;b++)for(unsigned s=0;s<9;s++)for(unsigned v=0;v<14;v++)for(unsigned q=1;q<14;q++){
        band=ints[b];subunit=ints[s];mssid=strings[v];printf("dw:%d\n",dw(strings[q]));cases++;
    }
    for(unsigned v=0;v<14;v++)for(mode=0;mode<3;mode++)for(unsigned q=1;q<14;q++){
        list=strings[v];printf("co:%d\n",co(strings[q]));printf("fh:%d\n",fh(strings[q]));cases+=2;
    }
    char buf[129];for(unsigned n=0;n<129;n++)for(unsigned p=0;p<12;p++){
        for(unsigned i=0;i<n;i++)buf[i]=(i+p)%(p+1)==0?' ':"ab\t\n"[(i+p)%4];
        buf[n]=0;list=buf;
        for(mode=0;mode<3;mode++)for(unsigned q=1;q<14;q++){
            printf("co:%d\n",co(strings[q]));printf("fh:%d\n",fh(strings[q]));cases+=2;
        }
    }
    fprintf(stderr,"%u interface lookup comparisons\n",cases);
}
