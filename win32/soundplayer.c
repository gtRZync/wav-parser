#include "soundplayer.h"
#include "wav_parser.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>


struct state__ {
    wav_file_t wav_file;
    WAVEFORMATEX format;
    HWAVEOUT hWaveOut;
    WAVEHDR waveHeader;
    bool is_playing;
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
    waveOutOpen(&snd->state->hWaveOut, WAVE_MAPPER, &snd->state->format, (DWORD_PTR)0, (DWORD_PTR)0, CALLBACK_NULL);
    waveOutPrepareHeader(snd->state->hWaveOut, &snd->state->waveHeader, sizeof(snd->state->waveHeader));
}

sound sound_init(const char* file_path) {
    state s = (state)malloc(sizeof(struct state__));
    if(!s) {
        fprintf(stderr, COLOR_RED "[ERROR] - malloc failed to allocate memory for sound state\n" COLOR_RESET);
        exit(EXIT_FAILURE);
    }
    memset(s, 0, sizeof(struct state__));
    sound snd;
    snd.file_path = NULL;
    size_t len = strlen(file_path);
    snd.file_path = (char*)malloc(len + 1);
    if(!snd.file_path) {
        fprintf(stderr, COLOR_RED "[ERROR] - malloc failed to allocate memory for file_path\n" COLOR_RESET);
        free(s);
        exit(EXIT_FAILURE);
    }
    strcpy(snd.file_path, file_path);
    snd.state = s;
    return snd;
}

void sound_load(sound *snd)
{  
    if(!wav_parse_file(snd->file_path, &snd->state->wav_file)) {
        sound_unload(snd);
        exit(EXIT_FAILURE);
    }
    WAVEFORMATEX_HDRinit(snd);
    prepareSoundData(snd);
}

void sound_unload(sound *snd)
{
    if (!snd) return;
    if(!snd->state && !snd->file_path) {
        fprintf(stdout, COLOR_YELLOW "\n[WARNING] - No free needed - Sound wasn't initialized.\n\n" COLOR_RESET);
        return;
    }
    if (snd->state) {
        waveOutUnprepareHeader(snd->state->hWaveOut, &snd->state->waveHeader, sizeof(snd->state->waveHeader));
        waveOutClose(snd->state->hWaveOut);
        free(snd->state);
        snd->state = NULL; 
        fprintf(stdout, COLOR_GREEN "\n[INFO] - Sound's state successfully unloaded!\n\n" COLOR_RESET);
    }
    if (snd->file_path) {
        free(snd->file_path);
        snd->file_path = NULL; 
        fprintf(stdout, COLOR_GREEN "\n[INFO] - Sound's file_path successfully freed!\n\n" COLOR_RESET);
    }
}

void play_sound(sound *snd)
{
    if(!snd->state->is_playing) {
        waveOutWrite(snd->state->hWaveOut, &snd->state->waveHeader, sizeof(snd->state->waveHeader));
        snd->state->is_playing = true;
    }
}

bool is_playing(sound *snd)
{
    return (snd->state->waveHeader.dwFlags & WHDR_DONE) == 0;
}

static void sound_reset_done_flag(sound* snd) {
    snd->state->waveHeader.dwFlags &= ~WHDR_DONE;
}

void CALLBACK waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR param1, DWORD_PTR param2) {
    
}
