#ifndef ID3V2_EDIT_H
#define ID3V2_EDIT_H

#include "detect.h"

int set_v2_frame(const char *filename,const TagInfo *info,const char *frameId,const char *value);

#endif