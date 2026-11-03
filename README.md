# MP3 Tag Reader

A command-line tool written in C that reads and edits ID3 metadata in MP3 files.

## Build

```
make
```

## Usage

```
./mp3tag [command] audio_file.mp3
./mp3tag [command] "new_info" audio_file.mp3
```

| Command | Action |
|---------|--------|
| `-v` | View the file's tag details |
| `-t` | Change the title |
| `-a` | Change the artist |
| `-A` | Change the album |
| `-y` | Change the year |
| `-c` | Change the comment |
| `-g` | Change the genre |
| `-T` | Change the track number |
| `-h` | Show help |

## Supported formats

- ID3v1 and ID3v1.1 (read and edit)
- ID3v2.2 and ID3v2.3 (read); ID3v2.3 (edit)
- ID3v2.4 (detected and read)

## Known limitations

- Tags using unsynchronisation or extended headers aren't handled.
- Only ID3v2.3 tags can be edited.
- Non-ASCII text is shown as `?`.
- Embedded album art is not displayed.
- When both tags exist, only ID3v2 is shown and edited.