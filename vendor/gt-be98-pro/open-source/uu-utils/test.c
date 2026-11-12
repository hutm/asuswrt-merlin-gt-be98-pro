/* SPDX-License-Identifier: GPL-2.0-only */
#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
int uu_model_check(void);
static int model,stage;
static char *product;
int get_model(void){assert(stage==0);stage=1;return model;}
char *get_productid(void){assert(stage==1);stage=2;return product;}
static unsigned cases;
static void run(int m,char *p){model=m;product=p;stage=0;int result=uu_model_check();assert(stage==2&&(result==0||result==1));printf("%d %u %d\n",m,cases++,result);}
int main(void)
{
 char *names[]={NULL,"","ExpertWiFi","ExpertWiFi-router","xExpertWiFix","expertwifi","RT-AC2200","RP-AC1900","RT-AX58U","RT-AX82U","TUF-AX5400","RT-AX5400","ZenWiFi_XD6","GT-BE98_PRO","RT-AX82U_V2","unknown"};
 for(int m=-2;m<=512;m++){
  for(unsigned i=0;i<sizeof(names)/sizeof(names[0]);i++){
   run(m,names[i]);if(!names[i])continue;
   char buffer[256];snprintf(buffer,sizeof(buffer),"%s ",names[i]);run(m,buffer);
   snprintf(buffer,sizeof(buffer)," %s",names[i]);run(m,buffer);
  }
  for(unsigned n=0;n<48;n++){char buffer[128];memset(buffer,'x',n);strcpy(buffer+n,"ExpertWiFi");run(m,buffer);}
 }
 for(unsigned i=0;i<sizeof(names)/sizeof(names[0]);i++){run(INT_MIN,names[i]);run(INT_MAX,names[i]);}
 fprintf(stderr,"%u model/product checks passed\n",cases);return 0;
}
