#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "id3v2_edit.h"

#define ID3V2_HEADER_SIZE 10
#define FRAME_HEADER_SIZE 10

static unsigned long read_syncsafe(const unsigned char *b){
    return ((unsigned long)(b[0]&0x7F)<<21) |
           ((unsigned long)(b[1]&0x7F)<<14) |
           ((unsigned long)(b[2]&0x7F)<<7)  |
            (unsigned long)(b[3]&0x7F);
}

static void write_syncsafe(unsigned char *b,unsigned long v){
    b[0]=(v>>21)&0x7F;
    b[1]=(v>>14)&0x7F;
    b[2]=(v>>7)&0x7F;
    b[3]=v&0x7F;
}

static unsigned long read_be32(const unsigned char *b){
    return ((unsigned long)b[0]<<24) |
           ((unsigned long)b[1]<<16) |
           ((unsigned long)b[2]<<8)  |
            (unsigned long)b[3];
}

static void write_be32(unsigned char *b,unsigned long v){
    b[0]=(v>>24)&0xFF;
    b[1]=(v>>16)&0xFF;
    b[2]=(v>>8)&0xFF;
    b[3]=v&0xFF;
}

/* Builds one complete v2.3 frame: ID, size, flags, encoding, body */
static unsigned char *build_frame(const char *frameId,const char *value,unsigned long *outLen){
    unsigned long textLen=strlen(value);
    unsigned long bodyLen;
    unsigned char *frame;
    int isComment=(strcmp(frameId,"COMM")==0);

    /* COMM body: encoding, language (3 bytes), empty description (00), text */
    if(isComment)
        bodyLen=1+3+1+textLen;
    else
        bodyLen=1+textLen;

    frame=malloc(FRAME_HEADER_SIZE+bodyLen);
    if(frame==NULL)
        return NULL;

    memcpy(frame,frameId,4);
    write_be32(&frame[4],bodyLen);
    frame[8]=0;
    frame[9]=0;
    frame[10]=0;    /* encoding: ISO-8859-1 */

    if(isComment){
        memcpy(&frame[11],"eng",3);
        frame[14]=0;
        memcpy(&frame[15],value,textLen);
    }else{
        memcpy(&frame[11],value,textLen);
    }

    *outLen=FRAME_HEADER_SIZE+bodyLen;
    return frame;
}

int set_v2_frame(const char *filename,const TagInfo *info,const char *frameId,const char *value){
    FILE *fp;
    long fileSizeL;
    size_t fileSize;
    unsigned char *data;
    unsigned char *newFrame;
    unsigned char *frames;
    unsigned char hdr[ID3V2_HEADER_SIZE];
    unsigned long newFrameLen=0;
    unsigned long tagEnd;
    unsigned long pos=ID3V2_HEADER_SIZE;
    unsigned long framesLen=0;
    int inserted=0;

    if(info->v2Major!=3){
        printf("ERROR: editing ID3v2.%d tags is not supported yet\n",info->v2Major);
        return -1;
    }

    fp=fopen(filename,"rb");
    if(fp==NULL)
        return -1;

    fseek(fp,0,SEEK_END);
    fileSizeL=ftell(fp);
    rewind(fp);
    fileSize=(size_t)fileSizeL;

    data=malloc(fileSize);
    if(data==NULL){
        fclose(fp);
        return -1;
    }
    if(fread(data,1,fileSize,fp)!=fileSize){
        fclose(fp);
        free(data);
        return -1;
    }
    fclose(fp);

    tagEnd=read_syncsafe(&data[6])+ID3V2_HEADER_SIZE;
    if(tagEnd>fileSize)
        tagEnd=fileSize;

    newFrame=build_frame(frameId,value,&newFrameLen);
    frames=malloc(tagEnd+newFrameLen);
    if(newFrame==NULL || frames==NULL){
        free(data);
        free(newFrame);
        free(frames);
        return -1;
    }

    /* Copy all frames except the one being replaced */
    while(pos+FRAME_HEADER_SIZE<=tagEnd && data[pos]!=0){
        unsigned long size=read_be32(&data[pos+4]);
        unsigned long frameLen=FRAME_HEADER_SIZE+size;

        if(pos+frameLen>tagEnd)
            break;

        if(memcmp(&data[pos],frameId,4)==0){
            if(!inserted){
                memcpy(frames+framesLen,newFrame,newFrameLen);
                framesLen+=newFrameLen;
                inserted=1;
            }
        }else{
            memcpy(frames+framesLen,&data[pos],frameLen);
            framesLen+=frameLen;
        }
        pos+=frameLen;
    }

    if(!inserted){
        memcpy(frames+framesLen,newFrame,newFrameLen);
        framesLen+=newFrameLen;
    }

    memcpy(hdr,"ID3",3);
    hdr[3]=3;
    hdr[4]=0;
    hdr[5]=0;
    write_syncsafe(&hdr[6],framesLen);

    /* Overwrite the file: header, frames, then the audio after the old tag */
    fp=fopen(filename,"wb");
    if(fp==NULL){
        free(data);
        free(newFrame);
        free(frames);
        return -1;
    }

    fwrite(hdr,1,ID3V2_HEADER_SIZE,fp);
    fwrite(frames,1,framesLen,fp);
    fwrite(&data[tagEnd],1,fileSize-tagEnd,fp);
    fclose(fp);

    free(data);
    free(newFrame);
    free(frames);
    return 0;
}