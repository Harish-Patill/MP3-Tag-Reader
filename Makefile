CC=gcc
CFLAGS=-Wall -Wextra -std=c99

mp3tag: main.c detect.c detect.h id3v1.c id3v1.h id3v2.c id3v2.h id3v2_edit.c id3v2_edit.h
	$(CC) $(CFLAGS) -o mp3tag main.c detect.c id3v1.c id3v2.c id3v2_edit.c

clean:
	rm -f mp3tag