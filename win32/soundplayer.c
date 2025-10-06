#include "soundplayer.h"
#include "wav_parser/wav_parser.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>

struct state__ {
    wav_file_t wav_file;
    WAVEFORMATEX format;
    HWAVEOUT hWaveOut;
    WAVEHDR waveHeader;
    /*bool is_playing*/
};

static void WAVEFORMATEX_HDRinit(const sound* snd) {
    wav_file_t* wav_file = &snd->state->wav_file;
    WAVEFORMATEX* format = &snd->state->format;
    WAVEHDR *wvHeader = &snd->state->waveHeader;
    //---------------Format Section-------------------
    format->wFormatTag = WAVE_FORMAT_PCM;
    format->nChannels = wav_file->header.num_channels;
    format->nSamplesPerSec = wav_file->header.sample_rate;
    format->nAvgBytesPerSec = (wav_file->header.sample_rate * wav_file->header.block_align);
    format->nBlockAlign = wav_file->header.block_align;
    format->wBitsPerSample = wav_file->header.bits_per_sample;
    format->cbSize = 0; 
    //---------------Header Section-------------------
    wvHeader->lpData = snd->state->wav_file.data;
    wvHeader->dwBufferLength = snd->state->wav_file.data_length; 
}

static void prepareSoundData(const sound* snd) {
    waveOutOpen(&snd->state->hWaveOut, WAVE_MAPPER, &snd->state->format, NULL, NULL, CALLBACK_NULL);
    waveOutPrepareHeader(snd->state->hWaveOut, &snd->state->waveHeader, sizeof(snd->state->waveHeader));
}

sound sound_init(const char* file_path) {
    state s = (state)malloc(sizeof(struct state__));
    memset(s, 0, sizeof(struct state__));
    return (sound) {.file_path=file_path, .state=s};
}

void sound_load(sound *snd)
{  
    if(!wav_parse_file(snd->file_path, &snd->state->wav_file)) {
        exit(EXIT_FAILURE);
    }
    WAVEFORMATEX_HDRinit(snd);
    prepareSoundData(snd);
}

void sound_unload(sound *snd)
{
    if (!snd) return;

    if (snd->state != NULL) {
        free(snd->state);
        snd->state = NULL; 
        fprintf(stdout, GREEN "\n[INFO] - Sound state successfully unload!\n\n" RESET);
    } else {
        fprintf(stdout, YELLOW "\n[WARNING] - No free needed - Sound state was not allocated.\n\n" RESET);
        return;
    }
    snd->file_path = "";
}

void play_sound(sound *snd)
{
    /*is_playing check*/
    waveOutWrite(snd->state->hWaveOut, &snd->state->waveHeader, sizeof(snd->state->waveHeader));
}
