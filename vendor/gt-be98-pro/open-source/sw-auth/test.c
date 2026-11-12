/* SPDX-License-Identifier: GPL-2.0-only */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <assert.h>
#include <limits.h>
extern char *get_auth_code(char *,char *,int);
static unsigned times,seeds,randoms;static time_t clock_value=1700000000;static int random_value=123;
time_t time(time_t *out){times++;if(out)*out=clock_value;return clock_value;}
void srand(unsigned seed){assert(seed==(unsigned)clock_value);seeds++;}
int rand(void){randoms++;return random_value;}
int main(void){
 char longvalue[515];memset(longvalue,'a',514);longvalue[514]=0;
 char *values[]={NULL,"","0","1","19","20","199","200","201","-1","-19","-20","-21","-199","-200","1700000000|test-key","123abc|x","|42|x","|||","  +123|x","9223372036854775807|x","9223372036854775808|x","-9223372036854775808|x",longvalue};
 unsigned caps[]={65,66,128,256,513};
 for(unsigned i=0;i<sizeof(values)/sizeof(values[0]);i++)for(unsigned j=0;j<5;j++)for(int e=0;e<2;e++){
  char out[530];memset(out,0x5a,sizeof(out));times=seeds=randoms=0;errno=e?ERANGE:0;
  char *result=get_auth_code(values[i],out,caps[j]);assert(result==out);assert(out[64]==0);
  for(unsigned k=caps[j];k<sizeof(out);k++)assert(out[k]==0x5a);
  printf("CASE %u %u %d %u %u %u %s\n",i,caps[j],e,times,seeds,randoms,out);
 }
 for(unsigned length=0;length<=513;length++)for(unsigned c=0;c<3;c++){
  char input[514],out[530];memset(input,'x',length);input[length]=0;if(length>=2){input[0]='1';input[1]='|';}
  unsigned cap=c==0?65:c==1?128:513;memset(out,0x5a,sizeof(out));times=seeds=randoms=0;errno=0;
  assert(get_auth_code(input,out,cap)==out);assert(out[64]==0);for(unsigned k=cap;k<sizeof(out);k++)assert(out[k]==0x5a);
  printf("LENGTH %u %u %u %u %u %s\n",length,cap,times,seeds,randoms,out);
 }
 for(int value=-401;value<=401;value++){
  char input[64],out[65];snprintf(input,sizeof(input),"%d|example|app",value);times=seeds=randoms=0;errno=0;
  assert(get_auth_code(input,out,sizeof(out))==out);printf("VALUE %d %u %u %u %s\n",value,times,seeds,randoms,out);
 }
 time_t clocks[]={0,1,1700000000,-1};int random_values[]={0,1,199,200,INT_MAX};char *fallbacks[]={NULL,"","0|key","invalid|key","9223372036854775808|key"};
 for(unsigned t=0;t<4;t++)for(unsigned r=0;r<5;r++)for(unsigned k=0;k<5;k++){
  char out[65];clock_value=clocks[t];random_value=random_values[r];times=seeds=randoms=0;errno=0;
  assert(get_auth_code(fallbacks[k],out,sizeof(out))==out);printf("RANDOM %u %u %u %u %u %u %s\n",t,r,k,times,seeds,randoms,out);
 }
 return 0;
}
