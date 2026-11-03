#ifndef DETECT_H
#define DETECT_H

/* Result of scanning a file for ID3 tags */
typedef struct{
    int hasV2;       /* 1 if ID3v2 tag found at start of file */
    int v2Major;     /* 2, 3 or 4 */
    int v2Revision;  /* revision byte from header */
    long v2Size;     /* total tag size in bytes, header included */
    int hasV1;       /* 1 if ID3v1 tag found in last 128 bytes */
}TagInfo;

/* Returns 0 on success, -1 if the file can't be opened */
int detect_tags(const char *filename,TagInfo *info);

#endif