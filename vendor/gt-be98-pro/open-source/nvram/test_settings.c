#define _POSIX_C_SOURCE 200809L
#include "settings.h"
#include <assert.h>
#include <stdlib.h>
static const char *names[]={"alpha","empty","line","spaced","extra","windows","dup","dup","last"};
static const char *values[]={"one","","first"," value ","a=b","crlf\r","first","last","no_newline"};
static int capture(void *context,const char *name,const char *value) {
    size_t *index=context;assert(*index<9);
    assert(!strcmp(names[*index],name) && !strcmp(values[*index],value));++*index;return 0;
}
static int fail(void *ctx,const char *name,const char *value) { (void)ctx;(void)name;(void)value;errno=EIO;return -1; }
int main(void) {
    FILE *f=tmpfile();size_t index=0;
    char data[128]={0};
    static const char records[]="alpha=one\0empty=\0line=first\nsecond\0";
    assert(f);
    fputs("alpha=one\nempty=\nline=first\nsecond\n spaced = value \n"
          "#comment=thing\nextra=a=b\nwindows=crlf\r\ndup=first\ndup=last\n"
          "no_separator\nlast=no_newline",f);
    rewind(f);
    assert(!nv_settings_load(f,capture,&index) && index==9);fclose(f);
    f=tmpfile();assert(f);assert(!nv_settings_write(f,records,sizeof records));
    rewind(f);assert(fread(data,1,sizeof data-1,f)>0);
    assert(!strcmp(data,"alpha=one\nempty=\nline=first\nsecond\n"));fclose(f);
    f=tmpfile();assert(f);
    assert(nv_settings_write(f,"missing terminator",18)==-1 && errno==EPROTO);
    assert(ftell(f)==0);fclose(f);
    f=tmpfile();assert(f);for(int i=0;i<1030;++i) fputc('x',f);fputc('\n',f);rewind(f);
    assert(nv_settings_load(f,capture,&index)==-1 && errno==EOVERFLOW);fclose(f);
    f=tmpfile();assert(f);fputs("key=value\n",f);rewind(f);
    assert(nv_settings_load(f,fail,NULL)==-1 && errno==EIO);fclose(f);
    puts("Settings codec: captured baseline load/write cases, oversized-line rejection and callback failure passed");
    return 0;
}
