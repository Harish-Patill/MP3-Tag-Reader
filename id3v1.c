#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "id3v1.h"

#define ID3V1_TAG_SIZE 128

typedef struct{
    const char *name;
    int code;
}GenreEntry;

static const GenreEntry genre_table[]={
    {"Blues",0},{"Classic Rock",1},{"Country",2},{"Dance",3},
    {"Disco",4},{"Hip-Hop",7},{"Jazz",8},{"Metal",9},
    {"Pop",13},{"Rap",15},{"Reggae",16},{"Rock",17},
    {"Techno",18},{"Alternative",20},{"Soundtrack",24},
    {"Ambient",26},{"Classical",32},{"Electronic",52},
    {"Folk",80},{"Latin",86},{NULL,0}
};

/* Case-insensitive comparison, so genre names match "pop" or "Pop" */
static int same_text(const char *a,const char *b){
    while(*a && *b){
        if(tolower((unsigned char)*a)!=tolower((unsigned char)*b))
            return 0;
        a++;
        b++;
    }
    return *a==*b;
}

/* Accepts a genre name from the table, or a plain number. Returns -1 if unknown */
static int genre_code(const char *s){
    int i;

    for(i=0;genre_table[i].name!=NULL;i++){
        if(same_text(genre_table[i].name,s))
            return genre_table[i].code;
    }
    if(s[0]>='0' && s[0]<='9' && atoi(s)<=255)
        return atoi(s);
    return -1;
}

/* Writes value into a fixed-size field, padding the rest with zeros */
static void put_field(unsigned char *dst,const char *value,int len){
    memset(dst,0,len);
    strncpy((char *)dst,value,len);
}

/* opt is the option letter: t,a,A,y,c,g,T */
int set_v1_field(const char *filename,char opt,const char *value){
    unsigned char buf[ID3V1_TAG_SIZE];
    FILE *fp;
    int code;

    fp=fopen(filename,"r+b");
    if(fp==NULL)
        return -1;

    if(fseek(fp,-ID3V1_TAG_SIZE,SEEK_END)!=0 ||
       fread(buf,1,ID3V1_TAG_SIZE,fp)!=ID3V1_TAG_SIZE ||
       memcmp(buf,"TAG",3)!=0){
        fclose(fp);
        return -1;
    }

    switch(opt){
        case 't': put_field(&buf[3],value,30);  break;
        case 'a': put_field(&buf[33],value,30); break;
        case 'A': put_field(&buf[63],value,30); break;
        case 'y': put_field(&buf[93],value,4);  break;
        case 'c':
            /* v1.1 comment is 28 bytes, v1.0 is 30 */
            if(buf[125]==0 && buf[126]!=0)
                put_field(&buf[97],value,28);
            else
                put_field(&buf[97],value,30);
            break;
        case 'T':
            code=atoi(value);
            if(code<0 || code>255){
                fclose(fp);
                printf("ERROR: track must be 0-255\n");
                return -1;
            }
            buf[125]=0;
            buf[126]=(unsigned char)code;
            break;
        case 'g':
            code=genre_code(value);
            if(code<0){
                fclose(fp);
                printf("ERROR: unknown genre '%s' for ID3v1\n",value);
                return -1;
            }
            buf[127]=(unsigned char)code;
            break;
        default:
            fclose(fp);
            return -1;
    }

    fseek(fp,-ID3V1_TAG_SIZE,SEEK_END);
    fwrite(buf,1,ID3V1_TAG_SIZE,fp);
    fclose(fp);
    return 0;
}

static void copy_field(char *dst,const unsigned char *src,int len){
    memcpy(dst,src,len);
    dst[len]='\0';
}

int read_id3v1(const char *filename,Id3v1Info *info){
    unsigned char buf[ID3V1_TAG_SIZE];
    FILE *fp;

    fp=fopen(filename,"rb");
    if(fp==NULL)
        return -1;

    if(fseek(fp,-ID3V1_TAG_SIZE,SEEK_END)!=0 ||
       fread(buf,1,ID3V1_TAG_SIZE,fp)!=ID3V1_TAG_SIZE ||
       memcmp(buf,"TAG",3)!=0){
        fclose(fp);
        return -1;
    }
    fclose(fp);

    copy_field(info->title,  &buf[3],30);
    copy_field(info->artist, &buf[33],30);
    copy_field(info->album,  &buf[63],30);
    copy_field(info->year,   &buf[93],4);

    /* ID3v1.1: byte 125 is 0 and byte 126 holds the track number */
    if(buf[125]==0 && buf[126]!=0){
        copy_field(info->comment,&buf[97],28);
        info->track=buf[126];
    }else{
        copy_field(info->comment,&buf[97],30);
        info->track=-1;
    }

    info->genre=buf[127];
    return 0;
}