#include "main_header.h"

TagData* create_tag_data() {
    TagData *data = (TagData *)malloc(sizeof(TagData));
    if(data) {
        data->version = NULL;
        data->title = NULL;
        data->artist = NULL;
        data->album = NULL;
        data->year = NULL;
        data->comment = NULL;
        data->genre = NULL;
        data->track = NULL;
    }
    return data;
}


void free_tag_data(TagData *data) {
    if(data) {
        free(data->version);
        free(data->title);
        free(data->artist);
        free(data->album);
        free(data->year);
        free(data->comment);
        free(data->genre);
        free(data->track);
        free(data);
    }
}
