# Simple WAV File Parser and Player in C

`c-wav-player` is a lightweight WAV file parser and Win32 sound player written in C. It reads standard RIFF/WAV headers, skips non-audio metadata chunks (such as smpl, LIST, etc.), and extracts raw PCM audio data for playback via the Windows waveOut API.

## Features

- Reads and validates WAV headers
- Supports 16-bit PCM, stereo and mono
- Skips unknown chunks safely (robust RIFF parsing)
- Prints header information and duration
- Play audio (via soundplayer.h)

## Usage Example 

### Wav Parser

```c
#include "wav_parser.h"

int main(int argc, char const *argv[])
{
    wav_file_t file;
    wav_init_file(&file);
    if(wav_parse_file("resources/sound/FlappyBird_Menu.wav", &file)) {
        wav_print_header(&file.header);
    }
    wav_free_file(&file);
    return 0;
}
```

### Win32 Soundplayer(using the wav parser)
```c
#include "win32/soundplayer.h"
#include "playsound_ui_demo.h"

int main(int argc, char const *argv[])
{
    sound snd = sound_init("resources/sound/bass-wiggle.wav");
    sound_load(&snd);
    play_sound(&snd);

    // Keep the main thread alive while the sound is playing.
    // This uses a simple UI demo that internally checks is_playing(snd).
    // Alternatively, a sleep loop or similar wait could be used instead.
    playsound_ui_demo(&snd);

    sound_unload(&snd);
    return 0;
}
```

## Example Output

### WAV parser (header data display)

|          Success Output                               |
|-------------------------------------------------------|
| ![Demo](resources/images/demo.png)                           |

|          Error Output                                 |
|-------------------------------------------------------|
| ![Error](resources/images/not_a_wav_err.png)                 |

### Win32 Soundplayer(using the wav parser)

|          Player Demo : Playing                        |
|-------------------------------------------------------|
| ![Demo](resources/images/player_demo.gif)                    |

|          Player Demo : End                            |
|-------------------------------------------------------|
| ![Demo](resources/images/player_demo_end.png)                    |


## License

This project is open source and free to use under the [MIT License](LICENSE).

