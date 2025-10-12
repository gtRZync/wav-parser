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
#include <stdio.h>
#include <stdlib.h> 
#include <stdint.h> 
#include <stdbool.h> 
#include <string.h> 

#define COLOR_RED             "\033[1;31m"
#define COLOR_GREEN           "\033[1;32m"
#define COLOR_YELLOW          "\033[1;33m"
#define COLOR_BLUE            "\033[1;34m"
#define COLOR_CYAN      "\033[1;36m"
#define COLOR_MAGENTA   "\033[1;35m"
#define COLOR_RESET     "\033[0m"

typedef struct wav_header_t {
    char RIFF[5];
    int32_t file_size;
    char WAVE[5];
    char fmt[5];                //! includes trailing null (usually "fmt ")
    int32_t chunk_size;         // size of format chunk (usually 16 for PCM)
    int16_t format_type;        // 1 = PCM
    int16_t num_channels;
    int32_t sample_rate;
    int32_t byte_rate;          //? sample_rate * num_channels * bits_per_sample / 8
    int16_t block_align;        //? num_channels * bits_per_sample / 8
    int16_t bits_per_sample;
    char data[5];               // "data"
    int32_t data_size;        //? size of the data section in bytes
} wav_header_t;

typedef struct wav_file_t
{
    wav_header_t header;
    uint8_t* data;
    int32_t data_length;
    int32_t samples;
}wav_file_t;

void wav_init_file(wav_file_t* wav_file) ;
void wav_free_file(wav_file_t* wav_file);
bool wav_parse_file(const char* filename, wav_file_t* wav_file);
void wav_print_header(const wav_header_t* header);




