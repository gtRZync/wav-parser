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

typedef struct wav_header_t {
    char RIFF[5];
    uint32_t file_size;
    char WAVE[5];
    char fmt[5];                //! includes trailing null (usually "fmt ")
    uint32_t chunk_size;         // size of format chunk (usually 16 for PCM)
    uint16_t format_type;        // 1 = PCM
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;          //? sample_rate * num_channels * bits_per_sample / 8
    uint16_t block_align;        //? num_channels * bits_per_sample / 8
    uint16_t bits_per_sample;
    char data[5];               // "data"
    uint32_t data_size;        //? size of the data section in bytes
} wav_header_t;

typedef struct wav_file_t
{
    wav_header_t header;
    uint8_t* data;
    uint32_t data_length;
    uint32_t samples;
}wav_file_t;

void wav_init_file(wav_file_t* wav_file);
void wav_free_file(wav_file_t* wav_file);
bool wav_parse_file(const char* filename, wav_file_t* wav_file);
void wav_print_header(const wav_header_t* header);




