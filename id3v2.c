#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "id3v2.h"

#define ID3V2_HEADER_SIZE 10
#define FRAME_HEADER_MAX  10

typedef struct{
    const char *id;
    const char *label;
}FrameName;

static const FrameName frame_names[]={
    {"TIT2","Title"},   {"TT2","Title"},
    {"TPE1","Artist"},  {"TP1","Artist"},
    {"TALB","Album"},   {"TAL","Album"},
    {"TYER","Year"},    {"TYE","Year"},
    {"TDRC","Year"},
    {"TCON","Genre"},   {"TCO","Genre"},
    {"TRCK","Track"},   {"TRK","Track"},
    {"TCOM","Composer"},{"TCM","Composer"},
    {"TPE2","Album artist"},{"TP2","Album artist"},
    {"TENC","Encoded by"},  {"TEN","Encoded by"},
    {"TPUB","Publisher"},   {"TPB","Publisher"},
    {NULL,NULL}
};

/* Returns the readable name for a frame ID of the given length, or NULL */
static const char *frame_label(const unsigned char *id,size_t idLen){
    int i;

    for(i=0;frame_names[i].id!=NULL;i++){
        if(strlen(frame_names[i].id)==idLen &&
           memcmp(frame_names[i].id,id,idLen)==0)
            return frame_names[i].label;
    }
    return NULL;
}

/* v2.2 sizes are 3 bytes, v2.3 are plain big-endian, v2.4 are syncsafe */
static unsigned long read_frame_size(const unsigned char *b,int major){
    if(major==2)
        return ((unsigned long)b[0]<<16) |
               ((unsigned long)b[1]<<8)  |
                (unsigned long)b[2];
    if(major==4)
        return ((unsigned long)(b[0]&0x7F)<<21) |
               ((unsigned long)(b[1]&0x7F)<<14) |
               ((unsigned long)(b[2]&0x7F)<<7)  |
                (unsigned long)(b[3]&0x7F);
    return ((unsigned long)b[0]<<24) |
           ((unsigned long)b[1]<<16) |
           ((unsigned long)b[2]<<8)  |
            (unsigned long)b[3];
}

/* Prints one text frame. First body byte is the text encoding */
static void print_text_frame(const unsigned char *id,size_t idLen,
                             const unsigned char *body,unsigned long len){
    const char *label;
    unsigned long i;
    unsigned long start=1;
    int utf16=0;
    int littleEndian=1;

    if(len<1)
        return;

    label=frame_label(id,idLen);
    if(label)
        printf("%-12s: ",label);
    else
        printf("%-12.*s: ",(int)idLen,(const char *)id);

    if(body[0]==1){
        utf16=1;
        if(len>=3 && body[1]==0xFE && body[2]==0xFF)
            littleEndian=0;
        if(len>=3 && ((body[1]==0xFF && body[2]==0xFE) || (body[1]==0xFE && body[2]==0xFF)))
            start=3;
    }

    if(!utf16){
        for(i=start;i<len && body[i]!='\0';i++)
            putchar(body[i]);
    }else{
        /* Only ASCII is printed for UTF-16 for now, other characters show as '?' */
        for(i=start;i+1<len;i+=2){
            unsigned char lo=littleEndian ? body[i]   : body[i+1];
            unsigned char hi=littleEndian ? body[i+1] : body[i];

            if(hi==0 && lo==0)
                break;
            if(hi==0)
                putchar(lo);
            else
                putchar('?');
        }
    }
    putchar('\n');
}

static const char *pic_type_name(unsigned char t){
    static const char *names[]={
        "Other","32x32 file icon","Other file icon","Cover (front)",
        "Cover (back)","Leaflet page","Media","Lead artist",
        "Artist","Conductor","Band/Orchestra","Composer",
        "Lyricist","Recording location","During recording",
        "During performance","Movie/video screen capture",
        "A bright coloured fish","Illustration","Band logo",
        "Publisher logo"
    };

    if(t<sizeof(names)/sizeof(names[0]))
        return names[t];
    return "Unknown";
}

/* offset is where the frame body starts in the file */
static void print_pic_frame(const unsigned char *body,unsigned long len,
                            unsigned long offset,int major){
    unsigned long i=1;
    unsigned char type;
    char mime[32]="";
    size_t m=0;
    int wide;

    if(len<4)
        return;

    wide=(body[0]==1 || body[0]==2);

    if(major==2){
        /* v2.2: 3-character image format such as JPG or PNG */
        snprintf(mime,sizeof(mime),"image/%c%c%c",
                 tolower(body[1]),tolower(body[2]),tolower(body[3]));
        i=4;
    }else{
        /* v2.3/v2.4: MIME type ends with 00 */
        while(i<len && body[i]!='\0'){
            if(m<sizeof(mime)-1)
                mime[m++]=body[i];
            i++;
        }
        mime[m]='\0';
        i++;
    }

    if(i>=len)
        return;
    type=body[i++];

    /* skip the description, ended by 00 (or 00 00 for Unicode) */
    if(wide){
        while(i+1<len && !(body[i]==0 && body[i+1]==0))
            i+=2;
        i+=2;
    }else{
        while(i<len && body[i]!='\0')
            i++;
        i++;
    }

    if(i>len)
        i=len;

    printf("Picture type: %s\n",pic_type_name(type));
    printf("Picture MIME: %s\n",mime);
    printf("Image offset: %lu bytes\n",offset+i);
    printf("Image size  : %lu bytes\n",len-i);
}

/* COMM body: encoding, 3-byte language, short description (00), then text */
static void print_comm_frame(const unsigned char *body,unsigned long len){
    unsigned long i=4;

    if(len<5)
        return;

    while(i<len && body[i]!='\0')
        i++;
    i++;

    printf("%-12s: ","Comment");
    for(;i<len && body[i]!='\0';i++)
        putchar(body[i]);
    putchar('\n');
}

int read_id3v2_frames(const char *filename,const TagInfo *info){
    unsigned char fh[FRAME_HEADER_MAX];
    size_t idLen;
    size_t fhSize;
    size_t sizeOff;
    unsigned long pos=ID3V2_HEADER_SIZE;
    unsigned long tagEnd=(unsigned long)info->v2Size;
    FILE *fp;

    /* v2.2 frame header is 6 bytes (3-char ID, 3-byte size),
       v2.3 and v2.4 are 10 bytes (4-char ID, 4-byte size, 2 flag bytes) */
    if(info->v2Major==2){
        idLen=3;
        sizeOff=3;
        fhSize=6;
    }else{
        idLen=4;
        sizeOff=4;
        fhSize=10;
    }

    fp=fopen(filename,"rb");
    if(fp==NULL)
        return -1;

    if(fseek(fp,ID3V2_HEADER_SIZE,SEEK_SET)!=0){
        fclose(fp);
        return -1;
    }

        while(pos+fhSize<=tagEnd){
        unsigned long size;
        unsigned long bodyOffset;
        unsigned char *body;
        int isPic;

        if(fread(fh,1,fhSize,fp)!=fhSize)
            break;

        /* A zero byte where a frame ID should be means padding starts here */
        if(fh[0]==0)
            break;

        size=read_frame_size(&fh[sizeOff],info->v2Major);
        pos+=fhSize;
        bodyOffset=pos;

        if(pos+size>tagEnd)
            break;
        if(size==0)
            continue;

        body=malloc((size_t)size);
        if(body==NULL){
            fclose(fp);
            return -1;
        }

        if(fread(body,1,(size_t)size,fp)!=size){
            free(body);
            break;
        }
        pos+=size;

        isPic=(idLen==4 && memcmp(fh,"APIC",4)==0) ||
              (idLen==3 && memcmp(fh,"PIC",3)==0);

        if(isPic){
            print_pic_frame(body,size,bodyOffset,info->v2Major);
        }else if(fh[0]=='T' && memcmp(fh,"TXX",3)!=0){
            /* Only text frames (T...) are printed, user-defined TXX/TXXX is skipped */
            print_text_frame(fh,idLen,body,size);
        }else if(idLen==4 && memcmp(fh,"COMM",4)==0){
            print_comm_frame(body,size);
        }

        free(body);
    }

    fclose(fp);
    return 0;
}