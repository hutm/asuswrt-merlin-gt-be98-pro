/* SPDX-License-Identifier: GPL-2.0-only */
#include "manufacturing.h"
#include <assert.h>
#include <errno.h>
#include <string.h>
static FILE *stream(const void *data,size_t size){FILE *f=tmpfile();assert(f);assert(fwrite(data,1,size,f)==size);rewind(f);return f;}
int main(void){
 const char data[]="alpha=one\0empty=\0et0macaddr=00:11:22:33:44:55\0\0ignored=yes\0";
 FILE *in=stream(data,sizeof data-1),*out=tmpfile(),*mac=stream("aa:bb:cc:dd:ee:ff",17);char text[256]={0};assert(out);
 fputs("previous=value\n",out);assert(!nv_mfg_append(in,out,mac));assert(ftell(mac)==0);rewind(out);fread(text,1,sizeof text-1,out);
 assert(!strcmp(text,"previous=value\nalpha=one\nempty=\net0macaddr=00:11:22:33:44:55\n"));fclose(in);fclose(out);
 in=stream("alpha=one\0\0",11);out=tmpfile();assert(out);assert(!nv_mfg_append(in,out,mac));rewind(out);memset(text,0,sizeof text);fread(text,1,sizeof text-1,out);assert(!strcmp(text,"alpha=one\net0macaddr=aa:bb:cc:dd:ee:ff\n"));fclose(in);fclose(out);fclose(mac);
 in=stream("unterminated",12);out=tmpfile();assert(nv_mfg_append(in,out,NULL)==-1&&errno==EPROTO&&ftell(out)==0);fclose(in);fclose(out);
 char long_record[1025];memset(long_record,'a',sizeof long_record);long_record[1024]=0;
 in=stream(long_record,sizeof long_record);out=tmpfile();assert(nv_mfg_append(in,out,NULL)==-1&&errno==EOVERFLOW&&ftell(out)==0);fclose(in);fclose(out);
 in=stream("",0);out=tmpfile();assert(!nv_mfg_append(in,out,NULL));fclose(in);fclose(out);
 puts("Manufacturing records: append format, terminal record, MAC fallback, truncation and bounds passed");return 0;
}
