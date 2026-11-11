/* SPDX-License-Identifier: GPL-2.0-only */
#include "names.h"
#include <stdarg.h>
#include <string.h>
int webapi_get_b(int id,char *buffer,size_t capacity)
{
 static const char *const names[]={"6e3f7d37","fdbca74d","68feaaa3","767cd197","537632ee","7ab9c272","89e792ac","5d097a12","1195e436","743191e3","de8feb47"};
 if(!buffer || !capacity)return 0;
 memset(buffer,0,capacity);
 if(id<1 || id>11)return 0;
 size_t n=capacity-1;if(n>8)n=8;
 memcpy(buffer,names[id-1],n);return (int)n;
}
void c(char *buffer,size_t capacity,...)
{
 if(!buffer || !capacity)return;
 size_t used=0;while(used<capacity && buffer[used])++used;
 if(used==capacity)return;
 va_list args;va_start(args,capacity);
 unsigned char ch;
 while((ch=(unsigned char)va_arg(args,int))!=0){
  if(used+1<capacity){buffer[used++]=(char)ch;buffer[used]=0;}
 }
 va_end(args);
}
