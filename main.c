#include <stdio.h>
#include <string.h>
#include "detect.h"
#include "id3v1.h"
#include "id3v2.h"
#include "id3v2_edit.h"

void help_display(){
    printf("Usage: \n");
    printf("./mp3tag [command] audio_file.mp3\n");
    printf("./mp3tag [command] \"new_info\" audio_file.mp3\n\n");
    printf("Commands:\n");
    printf("-v : to view the mp3 file's details\n");
    printf("-t : to change the title\n");
    printf("-a : to change the artist\n");
    printf("-A : to change the album\n");
    printf("-y : to change the year\n");
    printf("-c : to change the comment\n");
    printf("-g : to change the genre\n");
    printf("-T : to change the track\n");
    printf("-h : to display this help\n");
}

void print_v1_details(const Id3v1Info *v1){
    printf("Title   : %s\n",v1->title);
    printf("Artist  : %s\n",v1->artist);
    printf("Album   : %s\n",v1->album);
    printf("Year    : %s\n",v1->year);
    printf("Comment : %s\n",v1->comment);
    if(v1->track!=-1)
        printf("Track   : %d\n",v1->track);
    printf("Genre   : %d\n",v1->genre);
}

/* Maps an edit option to the ID3v2 frame it changes, or NULL */
static const char *edit_frame_id(const char *cmd){
    if(strcmp(cmd,"-t")==0) return "TIT2";
    if(strcmp(cmd,"-a")==0) return "TPE1";
    if(strcmp(cmd,"-A")==0) return "TALB";
    if(strcmp(cmd,"-y")==0) return "TYER";
    if(strcmp(cmd,"-c")==0) return "COMM";
    if(strcmp(cmd,"-g")==0) return "TCON";
    if(strcmp(cmd,"-T")==0) return "TRCK";
    return NULL;
}

static int load_tags(const char *filename,TagInfo *info){
    if(detect_tags(filename,info)!=0){
        printf("ERROR: cannot open '%s'\n",filename);
        return -1;
    }
    if(!info->hasV1 && !info->hasV2){
        printf("ERROR: no ID3 tag found in '%s'\n",filename);
        return -1;
    }
    return 0;
}

/* Prints the ID3v2 header line, then the start of the details section */
static void print_v2_header(const char *filename,const TagInfo *info){
    Id3v1Info v1;
    int v1Ok;

    printf("ID3v2.%d.%d found (tag size: %ld bytes)",
           info->v2Major,info->v2Revision,info->v2Size);

    if(info->hasV1){
        v1Ok=(read_id3v1(filename,&v1)==0);
        printf(", ID3v1%s also found",(v1Ok && v1.track!=-1) ? ".1" : "");
    }

    printf("\n\nID3v2 details:\n");
}

static int show_tags(const char *filename){
    TagInfo info;
    Id3v1Info v1;

    if(load_tags(filename,&info)!=0)
        return 1;

    if(info.hasV2){
        print_v2_header(filename,&info);
        read_id3v2_frames(filename,&info);
        return 0;
    }

    /* ID3v1 only */
    if(read_id3v1(filename,&v1)!=0){
        printf("ERROR: could not read ID3v1 tag\n");
        return 1;
    }

    printf("ID3v1%s found\n",v1.track!=-1 ? ".1" : "");
    print_v1_details(&v1);
    return 0;
}

static int edit_tag(const char *filename,const char *opt,const char *value){
    TagInfo info;
    const char *frameId=edit_frame_id(opt);

    if(load_tags(filename,&info)!=0)
        return 1;

    if(info.hasV2){
        if(set_v2_frame(filename,&info,frameId,value)!=0){
            printf("ERROR: could not update '%s'\n",filename);
            return 1;
        }
        printf("Updated %s to \"%s\"\n",frameId,value);
        return 0;
    }

    /* ID3v1-only file */
    if(set_v1_field(filename,opt[1],value)!=0){
        printf("ERROR: could not update ID3v1 tag\n");
        return 1;
    }
    printf("Updated ID3v1 %s to \"%s\"\n",opt,value);
    return 0;
}

int main(int argc,char* argv[]){
    const char *frameId;

    if(argc<2){
        help_display();
        return 1;
    }

    if(strcmp(argv[1],"-h")==0){
        help_display();
        return 0;
    }

    if(strcmp(argv[1],"-v")==0){
        if(argc<3){
            help_display();
            return 1;
        }
        return show_tags(argv[2]);
    }

    frameId=edit_frame_id(argv[1]);
    if(frameId!=NULL){
        if(argc<4){
            help_display();
            return 1;
        }
        return edit_tag(argv[3],argv[1],argv[2]);
    }

    printf("ERROR: unknown command '%s'\n",argv[1]);
    help_display();
    return 1;
}