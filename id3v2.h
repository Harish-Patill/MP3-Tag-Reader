#ifndef ID3V2_H
#define ID3V2_H

#include "detect.h"

int read_id3v2_frames(const char *filename,const TagInfo *info);

#endif