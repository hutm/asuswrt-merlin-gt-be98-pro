/* SPDX-License-Identifier: GPL-2.0-only */
#include "key.h"
#include <openssl/evp.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
static int digit(unsigned char c) {
    if(c>='0'&&c<='9')return c-'0';
    if(c>='a'&&c<='f')return c-'a'+10;
    if(c>='A'&&c<='F')return c-'A'+10;
    return -1;
}
int nv_pw_key(const char *salt,const char *product,const char *mac,char output[33]) {
    unsigned bytes[6],size=0;unsigned char digest[EVP_MAX_MD_SIZE];char material[128],key[33];
    static const char hex[]="0123456789abcdef";
    if(!product||!mac||!output||strlen(mac)!=17){errno=EINVAL;return -1;}
    for(unsigned i=0;i<6;++i){
        int high=digit((unsigned char)mac[i*3]),low=digit((unsigned char)mac[i*3+1]);
        if(high<0||low<0||(i<5&&mac[i*3+2]!=':')){errno=EINVAL;return -1;}
        bytes[i]=(unsigned)(high*16+low);
    }
    int n=snprintf(material,sizeof material,"%.32s%x%x%x%x%x%.63s%x",salt?salt:"",
        bytes[0]+bytes[3],bytes[1]*2,bytes[4]+bytes[5],bytes[2]+bytes[5],
        bytes[2]*bytes[4],product,bytes[0]*bytes[3]);
    if(n<0||(size_t)n>=sizeof material){errno=EOVERFLOW;return -1;}
    if(!EVP_Digest(material,(size_t)n,digest,&size,EVP_md5(),NULL)||size!=16){errno=EIO;return -1;}
    for(unsigned i=0;i<16;++i){key[i*2]=hex[digest[i]>>4];key[i*2+1]=hex[digest[i]&15];}
    key[32]=0;memcpy(output,key,sizeof key);return 0;
}
