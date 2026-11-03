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


int main(int argc,char *argv[]){

    // cla's validation
    if(argc < 2) {              // minimum 3 cla's are required
        display_help();
        return 1;
    }

    if(strcmp(argv[1], "-h") == 0){     // help display function as -h is passed
        display_help();
        return 0;
    } 
    else if(strcmp(argv[1], "-v") == 0 && argc == 3){       // mp3 files details will be displayed
        view_tags(argv[2]);     // passing the songs name that need to be displayed
    } 
    else if(strcmp(argv[1], "-e") == 0 && argc == 5){
        char *tag = argv[2];            // tag holds what to change
        char *filename = argv[3];       // filename holds the songs name
        char *value = argv[4];          // value holds the new value that will be replacing the value from the .mp3

        if(edit_tag(filename, tag, value) != 0){        // if the editing fails, program is terminated here.
            display_error("Failed to edit tag.");
            return 1;
        }
        printf("Tag edited successfully.\n");           // if the editing succeds display tag is edited
    }
    else{           
        display_help();
        return 1;
    }
    
    
    int version = check_id3_tag_presence(argv[2]);  // this will find the tag from the argv[2], which is song name

    switch (version){
        case 1:
            printf("ID3v1.0 detected\n\n");
            break;

        case 2:
            printf("ID3v1.1 detected\n\n");
            break;

        case 3:
            printf("ID3v2.2 detected\n\n");
            break;

        case 4:
            printf("ID3v2.3 detected\n\n");
            break;

        case 5:
            printf("ID3v2.4 detected\n\n");
            break;

        default:
            printf("No supported ID3 tag found\n");
            break;
    }

    return 0;
}
