#include "soundplayer.h"
#include "wav_parser/wav_parser.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>

struct state__ {
    wav_file_t wav_file;
    WAVEFORMATEX format;
    HWAVEOUT hWaveOut;
};

static void WAVEFORMATEXinit(const sound* _sound) {
    wav_file_t* wav_file = &_sound->state->wav_file;
    WAVEFORMATEX* format = &_sound->state->format;
    format->wFormatTag = WAVE_FORMAT_PCM;
    format->nChannels = wav_file->header.num_channels;
    format->nSamplesPerSec = wav_file->header.sample_rate;
    format->nAvgBytesPerSec = (wav_file->header.sample_rate * wav_file->header.block_align);
    format->nBlockAlign = wav_file->header.block_align;
    format->wBitsPerSample = wav_file->header.bits_per_sample;
}

sound sound_init(const char* file_path) {
    state s = (state)malloc(sizeof(struct state__));
    memset(s, 0, sizeof(struct state__));
    s->hWaveOut = rand() % 256;
    return (sound) {.file_path=file_path, .state=s};
}

void sound_load(sound *_sound)
{  
    if(!wav_parse_file(_sound->file_path, &_sound->state->wav_file)) {
        exit(EXIT_FAILURE);
    }
    WAVEFORMATEXinit(_sound);
    
}

void sound_unload(sound *_sound)
{
    if (!_sound) return;

    if (_sound->state != NULL) {
        free(_sound->state);
        _sound->state = NULL; 
        fprintf(stdout, GREEN "\n[INFO] - Sound state successfully unload!\n\n" RESET);
    } else {
        fprintf(stdout, YELLOW "\n[WARNING] - No free needed - Sound state was not allocated.\n\n" RESET);
        return;
    }
    _sound->file_path = "";
}
