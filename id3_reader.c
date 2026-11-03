#include "main_header.h"

// Copy metadata and remove trailing spaces
static char *copy_field(const unsigned char *source, size_t length){
    while (length > 0 && (source[length - 1] == ' ' || source[length - 1] == '\0')){
        length--;
    }

    char *result = malloc(length + 1);

    if (result == NULL){
        return NULL;
    }

    memcpy(result, source, length);
    result[length] = '\0';

    return result;
}

// Read ID3 tags from MP3 file
TagData* read_id3_tags(const char *filename){
    if (filename == NULL){
        display_error("Invalid filename.");
        return NULL;
    }

    FILE *fp = fopen(filename, "rb");

    if (fp == NULL){
        display_error("Unable to open MP3 file.");
        return NULL;
    }

    // Move to end of file
    if (fseek(fp, 0, SEEK_END) != 0){
        display_error("Unable to seek file.");
        fclose(fp);
        return NULL;
    }

    long file_size = ftell(fp);

    if (file_size < 128){
        display_error("File is too small for ID3v1 tags.");
        fclose(fp);
        return NULL;
    }

    // Read last 128 bytes
    if (fseek(fp, -128, SEEK_END) != 0){
        display_error("Unable to locate ID3 tag.");
        fclose(fp);
        return NULL;
    }

    unsigned char tag[128];

    if (fread(tag, 1, 128, fp) != 128){
        display_error("Failed to read ID3 tag.");
        fclose(fp);
        return NULL;
    }

    fclose(fp);

    // Check ID3 signature
    if (memcmp(tag, "TAG", 3) != 0){
        display_error("No ID3v1 tag found.");
        return NULL;
    }

    // Allocate TagData structure
    TagData *data = create_tag_data();

    if (data == NULL){
        display_error("Memory allocation failed.");
        return NULL;
    }

    // Check ID3v1.1 track number
    int is_v11 = (tag[125] == 0 && tag[126] != 0);

    // Assign metadata
    data->version = copy_field(
        (const unsigned char *)(is_v11 ? "ID3v1.1" : "ID3v1.0"),
        is_v11 ? 7 : 7
    );

    data->title = copy_field(tag + 3, 30);

    data->artist = copy_field(tag + 33, 30);

    data->album = copy_field(tag + 63, 30);

    data->year = copy_field(tag + 93, 4);

    // ID3v1.1 comment has 28 bytes
    data->comment = copy_field(
        tag + 97,
        is_v11 ? 28 : 30
    );

    // Store genre ID as a string
    char genre_id[4];

    snprintf(genre_id, sizeof(genre_id), "%u", tag[127]);

    data->genre = copy_field(
        (const unsigned char *)genre_id,
        strlen(genre_id)
    );

    // Track number exists only in ID3v1.1
    char track_id[4] = "";

    if (is_v11)
        snprintf(track_id, sizeof(track_id), "%u", tag[126]);

    data->track = copy_field((const unsigned char *)track_id, strlen(track_id));

    // Check memory allocation
    if (data->version   == NULL ||
        data->title     == NULL ||
        data->artist    == NULL ||
        data->album     == NULL ||
        data->year      == NULL ||
        data->comment   == NULL ||
        data->genre     == NULL ||
        data->track     == NULL){

        display_error("Memory allocation failed.");
        free_tag_data(data);

        return NULL;
    }

    return data;
}

// Display extracted metadata
void display_metadata(const TagData *data){
    if(data == NULL){
        display_error("No metadata available.");
        return;
    }

    printf("\n--------------- MP3 METADATA ---------------\n");

    printf("Version : %s\n", data->version);
    printf("Title   : %s\n", data->title);
    printf("Artist  : %s\n", data->artist);
    printf("Album   : %s\n", data->album);
    printf("Year    : %s\n", data->year);
    printf("Comment : %s\n", data->comment);
    printf("Genre ID: %s\n", data->genre);
    printf("Track   : %s\n", data->track);

    printf("----------------------------------------------\n");
}

// View tags
void view_tags(const char *filename){
    TagData *data = read_id3_tags(filename);
    if(data == NULL){
        display_error("Failed to read ID3 tags.");
        return;
    }
    display_metadata(data);
    free_tag_data(data);
}
