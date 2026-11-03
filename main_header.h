#ifndef MAIN_HEADER
#define MAIN_HEADER

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "id3_utils.h"
#include "id3_reader.h"
#include "id3_writer.h"


void display_error(const char *message);
int check_id3_tag_presence(const char *filename);

#endif // MAIN_HEADER