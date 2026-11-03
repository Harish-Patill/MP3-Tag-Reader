#ifndef ID3_UTILS_H
#define ID3_UTILS_H

typedef struct {
    char *version; /* Version of the ID3 tag */
    char *artist;  /* Artist of the song */
    char *title;   /* Title of the song */
    char *year;    /* Year of release */
    char *album;   /* Album name */
    char *comment; /* Comment */
    char *genre;   /* Genre */
} TagData;

TagData *create_tag_data(void);
void free_tag_data(TagData *data);

#endif // ID3_UTILS_H