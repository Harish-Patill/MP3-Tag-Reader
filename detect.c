#include <stdio.h>
#include <string.h>
#include "detect.h"

#define ID3V2_HEADER_SIZE 10
#define ID3V1_TAG_SIZE    128

static long syncsafe_to_long(const unsigned char *b){
    return ((long)(b[0]&0x7F)<<21) |
           ((long)(b[1]&0x7F)<<14) |
           ((long)(b[2]&0x7F)<<7)  |
            (long)(b[3]&0x7F);
}

int detect_tags(const char *filename,TagInfo *info){
    unsigned char hdr[ID3V2_HEADER_SIZE];
    unsigned char tag[3];
    FILE *fp;

    memset(info,0,sizeof(*info));

    fp=fopen(filename,"rb");
    if(fp==NULL)
        return -1;

    /* ID3v2: first 10 bytes of the file */
    if(fread(hdr,1,ID3V2_HEADER_SIZE,fp)==ID3V2_HEADER_SIZE &&
       memcmp(hdr,"ID3",3)==0 &&
       hdr[3]>=2 && hdr[3]<=4){
        info->hasV2=1;
        info->v2Major=hdr[3];
        info->v2Revision=hdr[4];
        info->v2Size=syncsafe_to_long(&hdr[6])+ID3V2_HEADER_SIZE;
    }

    /* ID3v1: first 3 bytes of the last 128 bytes */
    if(fseek(fp,-ID3V1_TAG_SIZE,SEEK_END)==0 &&
       fread(tag,1,3,fp)==3 &&
       memcmp(tag,"TAG",3)==0){
        info->hasV1=1;
    }

    fclose(fp);
    return 0;
}