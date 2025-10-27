/*
 * -------------------------------------------------------------
 *  c-wav-player: Simple WAV File Parser and Win32 Sound Player
 *  Author: Myson Dio (gtRZync)  
 *  License: MIT License
 *  Repository: https://github.com/gtRZync/c-wav-player
 *  
 *  Description:
 *    Lightweight parser for standard RIFF/WAV files and a sound
 *    playback system using Windows waveOut API. Skips metadata
 *    chunks (e.g. LIST, smpl), reads PCM audio data, and plays it.
 *
 *  See README.md for usage and LICENSE for distribution terms.
 * -------------------------------------------------------------
 */

/**
 * NOTE: Only the main thread may change (assign, null, or free) `snd`.
 * Threads may safely use or modify fields inside `*snd` if it’s non-null.
 * This check is thread-safe under that assumption.
 * if (!snd) return;
 */


#include "soundplayer.h"
#include "wav_parser.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include "log.h"

//!There's a bug when the playback starts (it's either cuz of the refs or critical sections misuse kinda). sometimes it does happends sometimes it doesn't
//!it's like a delayed playback start problem (like when the playback starts a tiny small part of the data buffer has alrdy been consumed)
//!: check play_sound
//TODO: add guards against invalid pointers usage 
//!Maybe add an InitSoundSystem that init a global lock and also keep tracks of every sound created to have better and safer access to ptr check
//* date: 25/10/2025, not so sure about the idea prior to this comment no more, but ion wanna remove it completely just yet 

static void CALLBACK waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR param1, DWORD_PTR param2);
static void sound_cleanup_on_fail(sound* snd);
static void sound_ctx_aqcuire(state internal);
static void sound_ctx_release(state internal);
static bool WaveOutOpFailed(MMRESULT mmResult, const char* fn_name);
static void WAVEFORMATEX_HDRinit(const sound* snd);
static void prepareSoundData(sound* snd);
static void unprepareSoundData(sound* snd);
static void unprepareSoundDataInCleanup(sound* snd);

struct state__ {
    wav_file_t wav_file;
    WAVEFORMATEX format;
    HWAVEOUT hWaveOut;
    WAVEHDR waveHeader;
    DWORD sndFlags;
    CRITICAL_SECTION lock;
    volatile LONG refcount;
    HANDLE hBufferDoneEvent; /*rename to hAllBufferDoneEvent for dbl buffering clarity*/
};

typedef enum Flags {
    SOUND_IS_INITIALIZED    = 0x00000001,
    SOUND_PLAYBACK_DONE     = 0x00000002,
    SOUND_IS_PLAYING        = 0x00000004,
    SOUND_WAV_PARSED        = 0x00000008,
    SOUND_LOOPING           = 0x0000000E, //?Even though 0xE is technically a combination of multiple bits, it represents one valid state (since a sound's is_done and playing flags are mutually exclusive )
}Flags;

//=================================================PUBLIC API IMPLEMENTATION==========================================================

sound *sound_init(const char* file_path) {
    state s = (state)malloc(sizeof(struct state__));
    if(!s) {
        Log(LOG_ERROR, "malloc failed to allocate memory for sound state\n");
        exit(EXIT_FAILURE);
    }
    memset(s, 0, sizeof(struct state__));
    sound *snd = (sound*)malloc(sizeof(sound));
    if(!snd) {
        Log(LOG_ERROR, "malloc failed to allocate memory for sound struct\n");
        free(s);
        exit(EXIT_FAILURE);
    }
    snd->file_path = NULL;
    size_t len = strlen(file_path);
    snd->file_path = (char*)malloc(len + 1);
    if(!snd->file_path) {
        Log(LOG_ERROR, "malloc failed to allocate memory for file_path\n");
        free(s);
        free(snd);
        exit(EXIT_FAILURE);
    }
    strcpy(snd->file_path, file_path);
    snd->state = s;
    return snd;
}

void sound_load(sound *snd)
{  
    //!could cause race cond if called after play_sound(...)
    if(!snd) return;
    if(snd->state->sndFlags & SOUND_WAV_PARSED) return; //!maybe log
    wav_init_file(&snd->state->wav_file);
    if(!wav_parse_file(snd->file_path, &snd->state->wav_file)) {
        sound_cleanup_on_fail(snd);
        exit(EXIT_FAILURE);
    }
    snd->state->sndFlags |= SOUND_WAV_PARSED;
    snd->state->hBufferDoneEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    InitializeCriticalSection(&snd->state->lock);
    WAVEFORMATEX_HDRinit(snd);
    prepareSoundData(snd);
}

void sound_unload(sound *snd)
{
    if (!snd) return; //!unsafe
    if(!snd->state && !snd->file_path) {
        Log(LOG_WARNING, "Only Sound's struct was freed - Sound's state wasn't initialized.\n\n");
        free(snd);
        return;
    }
    EnterCriticalSection(&snd->state->lock);
    if (snd->state) {
        unprepareSoundDataInCleanup(snd);
        wav_free_file(&snd->state->wav_file);
        LeaveCriticalSection(&snd->state->lock);
        DeleteCriticalSection(&snd->state->lock);
        LONG refCount = InterlockedCompareExchange((volatile LONG*)&snd->state->refcount, 0, 0);
        if(refCount == 0) { 
            free(snd->state);
            snd->state = NULL;
            if (snd->file_path) {
                free(snd->file_path);
                snd->file_path = NULL; 
                Log(LOG_INFO, "Sound's file_path successfully freed!\n\n");
            }
            free(snd);
        }
        Log(LOG_INFO, "Sound's state successfully unloaded!\n\n");
    }
}

/**
 * @brief Plays a parsed WAV sound.
 * 
 * @param snd Initialized sound struct.
 *! @warning Not thread-safe: `play_sound()` and `WaveOutProc` access state flags concurrently without synchronization, which may cause data races and undefined behavior.
 */
void play_sound(sound *snd)
{
    //TODO: yeah no vro pack it up and give every sound a critical section lock ✌🏻😭
    //TODO: maybe use double Buffering for replays
    if(!snd || !snd->state) return; //!unsafe access

    //!Enter Critical section , incr refcount, do things..,ResetEvent, decr ref, leave critical Section
    EnterCriticalSection(&snd->state->lock);
    sound_ctx_aqcuire(snd->state);
    if(!(snd->state->sndFlags & SOUND_IS_PLAYING)) {
        if((snd->state->sndFlags & SOUND_PLAYBACK_DONE) && (snd->state->waveHeader.dwFlags & WHDR_DONE)) {
            snd->state->sndFlags &= ~SOUND_PLAYBACK_DONE;
        }
        MMRESULT mmres = waveOutWrite(snd->state->hWaveOut, &snd->state->waveHeader, sizeof(WAVEHDR));
        if(WaveOutOpFailed(mmres, "waveOutWrite")) {
            unprepareSoundData(snd);
            prepareSoundData(snd);
            mmres = waveOutWrite(snd->state->hWaveOut, &snd->state->waveHeader, sizeof(WAVEHDR));
            if(WaveOutOpFailed(mmres, "waveOutWrite")) {
                unprepareSoundData(snd);
                Log(LOG_ERROR, "waveOutWrite failed after recovery attempt. Audio output unavailable. Aborting playback.\n");
                return;
            }
        }
        snd->state->sndFlags |= SOUND_IS_PLAYING;
        ResetEvent(snd->state->hBufferDoneEvent);
    }
    sound_ctx_release(snd->state);
    LeaveCriticalSection(&snd->state->lock);
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
    sound_ctx_aqcuire(snd->state);
    LONG result = InterlockedCompareExchange((volatile LONG*)&snd->state->sndFlags, 0, 0);
    sound_ctx_release(snd->state);
    return (result & SOUND_IS_PLAYING) != 0;
}

//=================================================PRIVATE UTILITY IMPLEMENTATION==========================================================

static void sound_ctx_aqcuire(state internal) {
    InterlockedIncrement(&internal->refcount);
}

static void sound_ctx_release(state internal) {
    InterlockedDecrement(&internal->refcount);
}

static bool WaveOutOpFailed(MMRESULT mmResult, const char* fn_name) {
    if (mmResult != MMSYSERR_NOERROR) {
        char errorText[MAXERRORLENGTH];
        waveOutGetErrorTextA(mmResult, errorText, MAXERRORLENGTH);
        Log(LOG_ERROR, "%s failed.\n", fn_name); //!use better errText
        Log(LOG_ERROR, "Reason: %s.\n", errorText); //!use better errText
        return true;
    }
    return false;
}

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
//!potentially unsafe usage lock before use 
static void prepareSoundData(sound* snd) {
    MMRESULT mmres = waveOutOpen(&snd->state->hWaveOut, WAVE_MAPPER, &snd->state->format, (DWORD_PTR)waveOutProc, (DWORD_PTR)snd, CALLBACK_FUNCTION);
    if(WaveOutOpFailed(mmres, "waveOutOpen")) {
        sound_cleanup_on_fail(snd);
        exit(EXIT_FAILURE);//! maybe not, will quit even if i was loading several sound and only one of em failed
    }
    mmres = waveOutPrepareHeader(snd->state->hWaveOut, &snd->state->waveHeader, sizeof(WAVEHDR));
    if(WaveOutOpFailed(mmres, "waveOutPrepareHeader")) {
        waveOutClose(snd->state->hWaveOut);
        sound_cleanup_on_fail(snd);
        exit(EXIT_FAILURE);//! maybe not, will quit even if i was loading several sound and only one of em failed
    }
}
//!potentially unsafe usage lock before use, also redudant code, just add a boolean flag wether to WaitForSingleObject or not
static void unprepareSoundData(sound* snd) {
    MMRESULT mmres = waveOutReset(snd->state->hWaveOut);
    if(WaveOutOpFailed(mmres, "waveOutUnprepareHeader")) {
        //? Continue anyway, maybe device already stopped/invalid
    }
    mmres = waveOutUnprepareHeader(snd->state->hWaveOut, &snd->state->waveHeader, sizeof(WAVEHDR));
    if(WaveOutOpFailed(mmres, "waveOutUnprepareHeader")) {
        //? Can't free buffer safely; proceed to close device
    }
    mmres = waveOutClose(snd->state->hWaveOut);
    if(WaveOutOpFailed(mmres, "waveOutClose")) {
        //? Nothing else to do — OS will reclaim resources on process exit
    }
}
//!this might be unsafe twan, whatchu saying ?
static void unprepareSoundDataInCleanup(sound* snd) {
    MMRESULT mmres = waveOutReset(snd->state->hWaveOut);
    if(WaveOutOpFailed(mmres, "waveOutUnprepareHeader")) {
        //? Continue anyway, maybe device already stopped/invalid
    }
    //!wait for the callback_func to receive WOM_DONE
    WaitForSingleObject(snd->state->hBufferDoneEvent, INFINITE);

    mmres = waveOutUnprepareHeader(snd->state->hWaveOut, &snd->state->waveHeader, sizeof(WAVEHDR));
    if(WaveOutOpFailed(mmres, "waveOutUnprepareHeader")) {
        //? Can't free buffer safely; proceed to close device
    }
    mmres = waveOutClose(snd->state->hWaveOut);
    if(WaveOutOpFailed(mmres, "waveOutClose")) {
        //? Nothing else to do — OS will reclaim resources on process exit
    }
}


static void sound_cleanup_on_fail(sound* snd) {
    if (!snd) return;
    if(!snd->state && !snd->file_path) {
        Log(LOG_WARNING, "Only Sound's struct was freed - Sound's state wasn't initialized.\n\n");
        free(snd);
        return;
    }
    if (snd->state) {
        free(snd->state);
        snd->state = NULL;
        Log(LOG_INFO, "Sound's state successfully unloaded!\n\n");
    }
    if (snd->file_path) {
        free(snd->file_path);
        snd->file_path = NULL; 
        Log(LOG_INFO, "Sound's file_path successfully freed!\n\n");
    }
    if (snd) {
        free(snd);
        snd = NULL; 
        Log(LOG_INFO, "Sound's struct successfully freed!\n\n");
    }
}

static void CALLBACK waveOutProc(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR param1, DWORD_PTR param2) {
    switch(uMsg) {
        case WOM_DONE:
        {
            sound* snd = (sound*)dwInstance;

            if(snd) {
                sound_ctx_aqcuire(snd->state);
                InterlockedAnd((volatile LONG*)&snd->state->sndFlags, ~SOUND_IS_PLAYING);
                InterlockedOr((volatile LONG*)&snd->state->sndFlags, SOUND_PLAYBACK_DONE);
                SetEvent(snd->state->hBufferDoneEvent);//! may be unsafe to access, consider using locks
                sound_ctx_release(snd->state);
            }
        }
    }
}