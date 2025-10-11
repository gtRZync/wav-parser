#include "soundplayer.h"
#include "wav_parser.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>

//TODO: add guards against invalid pointers usage 

static void CALLBACK waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR param1, DWORD_PTR param2);

struct state__ {
    wav_file_t wav_file;
    WAVEFORMATEX format;
    HWAVEOUT hWaveOut;
    WAVEHDR waveHeader;
    DWORD sndFlags;
};

typedef enum Flags {
    SOUND_PLAYBACK_DONE = 0x00000001,
    SOUND_IS_PLAYING = 0x00000002,
    SOUND_WAV_PARSED = 0x00000004,
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

static void prepareSoundData(sound* snd) {
    //TODO: use dwInstance to add sound ctx and manage flags such as SOUND_PLAYBACK_DONE, SOUND_IS_PLAYING...etc
    waveOutOpen(&snd->state->hWaveOut, WAVE_MAPPER, &snd->state->format, (DWORD_PTR)waveOutProc, (DWORD_PTR)snd, CALLBACK_FUNCTION);
    waveOutPrepareHeader(snd->state->hWaveOut, &snd->state->waveHeader, sizeof(WAVEHDR));
}

static void unprepareSoundData(sound* snd) {
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
    if(snd->state->sndFlags & SOUND_WAV_PARSED) return; //!maybe log

    if(!wav_parse_file(snd->file_path, &snd->state->wav_file)) {
        sound_unload(snd);
        exit(EXIT_FAILURE);
    }
    snd->state->sndFlags |= SOUND_WAV_PARSED;
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
 * @brief Plays a parsed WAV sound.
 * 
 * @param snd Initialized sound struct.
 *! @warning Not thread-safe: `play_sound()` and `WaveOutProc` access `sndFlags` concurrently without synchronization, which may cause data races and undefined behavior.
 */
void play_sound(sound *snd)
{
    //TODO: use CriticalSections, or Atomic functions such as InterlockedOr, InterlockedAnd, etc.,
    //TODO: maybe use double Buffering for replays
    if(!(snd->state->sndFlags & SOUND_IS_PLAYING)) {
        if((snd->state->sndFlags & SOUND_PLAYBACK_DONE) && (snd->state->waveHeader.dwFlags & WHDR_DONE)) {
            snd->state->sndFlags &= ~SOUND_PLAYBACK_DONE;
        }
        waveOutWrite(snd->state->hWaveOut, &snd->state->waveHeader, sizeof(WAVEHDR));
        snd->state->sndFlags |= SOUND_IS_PLAYING;
    }
}

/**
 * @brief Checks whether the audio buffer is still playing.
 *
 * This function reads the `dwFlags` member atomically using `InterlockedCompareExchange`
 * to ensure a thread-safe read without modifying the value.
 *
 * @returns `true` if playback is ongoing (`WHDR_DONE` not set); `false` if playback is complete.
 *
 * @note This function performs an atomic read of `dwFlags` but does not provide
 *       full synchronization for concurrent updates beyond the atomicity of the read.
 */
bool is_playing(sound *snd)
{
    LONG result = InterlockedCompareExchange(&snd->state->waveHeader.dwFlags, 0, 0);
    return (result & WHDR_DONE) == 0;
}

static void sound_reset_done_flag(sound* snd) {
    snd->state->waveHeader.dwFlags &= ~WHDR_DONE;
}

static void CALLBACK waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR param1, DWORD_PTR param2) {
    switch(uMsg) {
        case WOM_DONE:
        {
            sound* snd = (sound*)dwInstance;

            snd->state->sndFlags &= ~SOUND_IS_PLAYING;
            snd->state->sndFlags |= SOUND_PLAYBACK_DONE;
        }
    }
}
