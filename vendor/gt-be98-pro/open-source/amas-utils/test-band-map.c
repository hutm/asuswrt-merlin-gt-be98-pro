#include <assert.h>
#include <dlfcn.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
static int counts[3], calls, units[16], indexes[16], channel, set_channel;
int get_wl_count(void) { int v=counts[calls<2?calls:2];calls++;printf("count:%d;",v);return v; }
int nvram_get_int(const char *key) {
    printf("get:%s;",key);
    for(int i=0;i<16;i++) {
        char k[64];snprintf(k,sizeof(k),"amas_wlc%d_index",i);if(!strcmp(k,key))return indexes[i];
        snprintf(k,sizeof(k),"amas_wlc%d_unit",i);if(!strcmp(k,key))return units[i];
    }
    assert(0);return 0;
}
void get_control_channel(int unit,int *ch,int *bw,int *side) {
    assert(*ch==0&&*bw==0&&*side==0);printf("channel:%d:%d:%d;",unit,channel,set_channel);
    if(set_channel)*ch=channel;
    *bw=123;*side=-2;
}
int main(int argc,char **argv) {
    assert(argc==2);void *h=dlopen(argv[1],RTLD_LAZY|RTLD_LOCAL);assert(h);
    int (*to_unit)(int)=dlsym(h,"get_unit_by_wlc_bandindex"),(*to_index)(int)=dlsym(h,"get_wlc_bandindex_by_unit"),(*type)(int,int)=dlsym(h,"get_type_by_channel");assert(to_unit&&to_index&&type);
    unsigned cases=0,seed=0x9383;
    int edge[]={INT_MIN,-1,0,1,4,8,15,INT_MAX};
    for(unsigned trial=0;trial<10000;trial++) {
        for(int i=0;i<16;i++) { seed=seed*1664525u+1013904223u;units[i]=edge[seed>>29];seed=seed*1664525u+1013904223u;indexes[i]=edge[seed>>29]; }
        for(int i=0;i<3;i++){seed=seed*1664525u+1013904223u;counts[i]=(int)(seed%18)-2;}
        for(unsigned q=0;q<8;q++) {calls=0;printf("unit:%d\n",to_unit(edge[q]));calls=0;printf("index:%d\n",to_index(edge[q]));cases+=2;}
    }
    int bands[]={2,1,4};
    for(unsigned u=0;u<8;u++)for(unsigned b=0;b<3;b++)for(channel=-10;channel<270;channel++)for(set_channel=0;set_channel<2;set_channel++) {printf("type:%d\n",type(edge[u],bands[b]));cases++;}
    for(unsigned u=0;u<8;u++)for(unsigned b=0;b<3;b++)for(unsigned c=0;c<8;c++) {channel=edge[c];set_channel=1;printf("type:%d\n",type(edge[u],bands[b]));cases++;}
    fprintf(stderr,"%u band mapping comparisons\n",cases);
}
