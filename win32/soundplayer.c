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
    DWORD sndFlags;
};

typedef enum Flags {
    SOUND_PLAYBACK_DONE = 0x00000001,
    SOUND_IS_PLAYING = 0x00000002
}Flags;

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
    //TODO: use dwInstance to add sound ctx and manage flags such as SOUND_PLAYBACK_DONE, SOUND_IS_PLAYING...etc
    waveOutOpen(&snd->state->hWaveOut, WAVE_MAPPER, &snd->state->format, (DWORD_PTR)0, (DWORD_PTR)0, CALLBACK_NULL);
    waveOutPrepareHeader(snd->state->hWaveOut, &snd->state->waveHeader, sizeof(WAVEHDR));
}

static void unprepareSoundData(const sound* snd) {
    waveOutUnprepareHeader(snd->state->hWaveOut, &snd->state->waveHeader, sizeof(WAVEHDR));
    waveOutClose(snd->state->hWaveOut);
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
        unprepareSoundData(snd);
        free(snd->state);
        snd->state = NULL; 
        fprintf(stdout, COLOR_GREEN "\n[INFO] - Sound's state successfully unloaded!\n\n" COLOR_RESET);
    }
    if (snd->file_path) {
        free(snd->file_path);
        snd->file_path = NULL; 
        fprintf(stdout, COLOR_GREEN "[INFO] - Sound's file_path successfully freed!\n\n" COLOR_RESET);
    }
}

/**
 * @brief plays a sound from a parsed wav sound
 * 
 * @param snd initialized sound struct
 *! @warning `play_sound()` runs in the main thread, while the `WaveOutProc` callback is executed by the Windows audio system in a separate thread. Both functions access `sndFlags` without synchronization.
 *! This introduces a potential data race, which can lead to undefined behavior. Even if it appears to work reliably during simple flag checks, this is not guaranteed to be safe.
 */
void play_sound(sound *snd)
{
    if(!(snd->state->sndFlags & SOUND_IS_PLAYING)) {
        waveOutWrite(snd->state->hWaveOut, &snd->state->waveHeader, sizeof(WAVEHDR));
        snd->state->sndFlags |= SOUND_IS_PLAYING;
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
    switch(uMsg) {
        case WOM_DONE:
        {

        }
    }
}
