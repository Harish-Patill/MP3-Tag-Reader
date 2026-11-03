#ifndef ID3_READER_H
#define ID3_READER_H

#include "id3_utils.h"

TagData *read_id3_tags(const char *filename);
void display_metadata(const TagData *data);
void view_tags(const char *filename);

#endif // ID3_READER_H