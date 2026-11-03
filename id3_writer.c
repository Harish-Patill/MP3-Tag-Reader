#include "main_header.h"

// Copy string into ID3 field with zero padding
static void copy_to_tag(unsigned char *destination,const char *source,size_t size){
    memset(destination, 0, size);

    if (source != NULL){
        size_t length = strlen(source);

        if (length > size)
            length = size;

        memcpy(destination, source, length);
    }
}

// Allocate and replace a metadata string
static int update_field(char **field, const char *value){
    if (value == NULL){
        return 1;
    }

    char *new_value = malloc(strlen(value) + 1);

    if(new_value == NULL){
        return 1;
    }

    strcpy(new_value, value);

    free(*field);
    *field = new_value;

    return 0;
}

// Write ID3v1 metadata into an MP3 file
int write_id3_tags(const char *filename, const TagData *data){
    if (filename == NULL || data == NULL){
        display_error("Invalid filename or metadata.");
        return 1;
    }

    FILE *fp = fopen(filename, "r+b");

    if (fp == NULL){
        display_error("Unable to open MP3 file.");
        return 1;
    }

    // Find end of file
    if (fseek(fp, 0, SEEK_END) != 0){
        display_error("Unable to seek file.");
        fclose(fp);
        return 1;
    }

    long file_size = ftell(fp);

    if (file_size < 0){
        display_error("Unable to determine file size.");
        fclose(fp);
        return 1;
    }

    unsigned char tag[128] = {0};

    // ID3v1 signature
    memcpy(tag, "TAG", 3);

    // Fill metadata fields
    copy_to_tag(tag + 3, data->title, 30);
    copy_to_tag(tag + 33, data->artist, 30);
    copy_to_tag(tag + 63, data->album, 30);
    copy_to_tag(tag + 93, data->year, 4);
    copy_to_tag(tag + 97, data->comment, 30);

    // Genre ID (numeric string)
    int genre = 255;

    if(data->genre != NULL && data->genre[0] != '\0'){
        char *end;
        long value = strtol(data->genre, &end, 10);

        if (*end == '\0' && value >= 0 && value <= 255) genre = (int)value;
           
        else{
            display_error("Genre must be a numeric ID from 0 to 255.");
            fclose(fp);
            return 1;
        }
    }

    tag[127] = (unsigned char)genre;

    // Check whether an ID3v1 tag already exists
    int existing_tag = 0;

    if(file_size >= 128){
        unsigned char old_tag[128];

        if (fseek(fp, -128, SEEK_END) == 0 && fread(old_tag, 1, 128, fp) == 128){
            if (memcmp(old_tag, "TAG", 3) == 0)
                existing_tag = 1;
        }
    }

    // Overwrite existing tag or append a new tag
    if(existing_tag){
        if (fseek(fp, -128, SEEK_END) != 0){
            display_error("Unable to locate existing tag.");
            fclose(fp);
            return 1;
        }
    }
    else{
        if (fseek(fp, 0, SEEK_END) != 0){
            display_error("Unable to move to file end.");
            fclose(fp);
            return 1;
        }
    }

    if(fwrite(tag, 1, 128, fp) != 128){
        display_error("Failed to write ID3 tags.");
        fclose(fp);
        return 1;
    }

    if (fclose(fp) != 0){
        display_error("Failed to close MP3 file.");
        return 1;
    }

    return 0;
}


// Edit a particular metadata field
int edit_tag(const char *filename, const char *tag, const char *value){
    if (filename == NULL || tag == NULL || value == NULL){
        display_error("Invalid editing arguments.");
        return 1;
    }

    // Read existing metadata
    TagData *data = read_id3_tags(filename);

    // If no ID3v1 tag exists, create empty metadata
    if(data == NULL){
        data = create_tag_data();

        if(data == NULL){
            display_error("Memory allocation failed.");
            return 1;
        }
    }

    int result = 0;

    // Select the metadata field
    if(strcmp(tag, "-t") == 0){         // updating tile
        result = update_field(&data->title, value);
    }
    else if(strcmp(tag, "-a") == 0){    // updating artist
        result = update_field(&data->artist, value);
    }
    else if(strcmp(tag, "-A") == 0){    // updating album_name
        result = update_field(&data->album, value);
    }
    else if(strcmp(tag, "-y") == 0){    // updating year_of_release
        result = update_field(&data->year, value);
    }
    else if(strcmp(tag, "-c") == 0){    // updating comment
        result = update_field(&data->comment, value);
    }
    else if(strcmp(tag, "-g") == 0){    // updating genre
        result = update_field(&data->genre, value);
    }
    else if(strcmp(tag, "-T") == 0){    // track editing
        display_error("Track editing is not supported by the current TagData structure.");
        free_tag_data(data);
        return 1;
    }
    else{       // invalid tag option
        display_error("Invalid tag option.");
        free_tag_data(data);
        return 1;
    }

    if(result != 0){
        display_error("Unable to update metadata.");
        free_tag_data(data);
        return 1;
    }

    // Save updated metadata
    result = write_id3_tags(filename, data);

    // Release allocated memory
    free_tag_data(data);

    return result;
}