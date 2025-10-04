#include "soundplayer.h"
#include "wav_parser/wav_parser.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>

sound sound_init(const char* filename) {
    wav_file_t wav_file;
    wav_init_file(&wav_file);
    return (sound) {.filename=filename, .wav_file=wav_file};
}

void sound_load(const sound *_sound)
{
    if(!wav_parse_file(_sound->filename, _sound->wav_file)) {
        exit(EXIT_FAILURE);
    }
    
}
