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

#pragma once
#include <stdbool.h>

typedef struct state__ *state;

typedef struct sound {
    char* file_path;
    state state;
} sound;

sound sound_init(const char* file_path);
void  sound_load(sound* _sound);
void  sound_unload(sound* _sound);
void  play_sound(sound* _sound);
bool  is_playing(sound* snd);