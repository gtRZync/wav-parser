# Simple WAV File Parser in C

A lightweight WAV file parser written in C that reads standard RIFF/WAV headers,
supports skipping extra metadata chunks (like `smpl`, `LIST`), and extracts audio data info.

## Features

- Reads and validates WAV headers
- Supports 16-bit PCM, stereo and mono
- Skips unknown chunks safely (robust RIFF parsing)
- Prints header information and duration

## Usage Example 

### Wav Parser

```c
#include "wav_parser.h"

int main(int argc, char const *argv[])
{
    wav_file_t file;
    wav_init_file(&file);
    if(wav_parse_file("resources/FlappyBird_Menu.wav", &file)) {
        wav_print_header(&file.header);
    }
    wav_free_file(&file);
    return 0;
}
```

### Win32 Soundplayer(using the wav parser)
```c
#include "win32/soundplayer.h"

//temporary code 
int main(int argc, char const *argv[])
{
    sound snd = sound_init("resources/FlappyBird_Menu.wav");
    sound_load(&snd);
    play_sound(&snd);
    sleep(41250); 
    sound_unload(&snd);
    return 0;
}
```

## Example Output

| Terminal Success Output                               |
|-------------------------------------------------------|
| ![Demo](resources/demo.png)                           |

| Terminal Error Output                                 |
|-------------------------------------------------------|
| ![Error](resources/not_a_wav_err.png)                 |

## License

This project is open source and free to use under the [MIT License](LICENSE).

