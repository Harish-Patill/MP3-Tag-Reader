#include "main_header.h"

// will check for what id3 tag is present in the .mp3 file
int check_id3_tag_presence(const char *filename){
    if(filename == NULL){   // end if the file name is null
        return 0;
    }

    FILE *fp=fopen(filename, "rb");     // open in read binary mode

    if(fp == NULL){
        display_error("Unable to open file.");
        return 0;
    }

    unsigned char header[10];
    unsigned char tag[128];

    // Check ID3v2 at the beginning
    if (fread(header, 1, 10, fp) == 10){        // ID3v2 has header is exactly 10bytes, so it checks if its ID3v2 
        if (memcmp(header, "ID3", 3) == 0){     // checks whether the first three bytes are "ID3", if so then its ID3v2, else go below to check for ID3v1
            int version = header[3];

            fclose(fp);

            if(version == 2){
                return 3;
            }
            else if(version == 3){
                return 4;
            }
            else if(version == 4){
                return 5;
            }
            return 0;
        }
    }

    // Check ID3v1 at the end
    if ((fseek(fp, -128, SEEK_END) == 0) && (fread(tag, 1, 128, fp) == 128)){
        if (memcmp(tag, "TAG", 3) == 0){
            fclose(fp);

            // Detect ID3v1.1
            if (tag[125] == 0 && tag[126] != 0)
                return 2;

            return 1;
        }
    }

    fclose(fp);

    return 0;
}
