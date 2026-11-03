#ifndef ID3V1_H
#define ID3V1_H

typedef struct{
    char title[31];
    char artist[31];
    char album[31];
    char year[5];
    char comment[31];
    int track;       /* -1 if not ID3v1.1 */
    int genre;       /* genre number, name lookup comes later */
}Id3v1Info;

int read_id3v1(const char *filename,Id3v1Info *info);
int set_v1_field(const char *filename,char opt,const char *value);

#endif