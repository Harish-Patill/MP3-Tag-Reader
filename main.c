#include "main_header.h"

// help function to be displayed for the user to understand the usage
void display_help(){
    printf("\nUsage: \n");
    printf("1. To view details:  ./mp3tag -v <audio_file.mp3>\n");
    printf("2. To edit details:  ./mp3tag -e <commands> <audio_file.mp3> <new_value>\n\n");
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


// Print the detected ID3 version of a file
static void print_tag_version(const char *filename){
    int version = check_id3_tag_presence(filename);

    switch(version){
        case 1: printf("ID3v1.0 detected\n\n"); break;
        case 2: printf("ID3v1.1 detected\n\n"); break;
        case 3: printf("ID3v2.2 detected\n\n"); break;
        case 4: printf("ID3v2.3 detected\n\n"); break;
        case 5: printf("ID3v2.4 detected\n\n"); break;
        default: printf("No supported ID3 tag found\n"); break;
    }
}

int main(int argc,char *argv[]){
    if(argc < 2){
        display_help();
        return 1;
    }

    if(strcmp(argv[1],"-h")==0){
        display_help();
        return 0;
    }

    if(strcmp(argv[1],"-v")==0 && argc==3){
        view_tags(argv[2]);
        print_tag_version(argv[2]);
        return 0;
    }

    if(strcmp(argv[1],"-e")==0 && argc==5){
        // argv[2] = tag flag, argv[3] = file, argv[4] = new value
        if(edit_tag(argv[3],argv[2],argv[4]) != 0){
            display_error("Failed to edit tag.");
            return 1;
        }
        printf("Tag edited successfully.\n");
        return 0;
    }

    display_help();
    return 1;
}