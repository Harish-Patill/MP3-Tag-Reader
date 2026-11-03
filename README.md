# MP3 Tag Reader

A command-line tool written in C that reads and edits ID3 metadata in MP3 files.

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
