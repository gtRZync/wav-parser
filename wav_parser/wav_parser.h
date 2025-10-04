#pragma once
#include <stdio.h>
#include <stdlib.h> 
#include <stdint.h> 
#include <stdbool.h> 
#include <string.h> 

#define RED     "\e[1;31m"
#define GREEN   "\e[1;32m"
#define YELLOW  "\e[1;33m"
#define BLUE    "\e[1;34m"
#define CYAN    "\e[1;36m"
#define RESET   "\e[0m"

typedef struct WAV_HEADER {
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
} WAV_HEADER;

typedef struct WAVFile_t
{
    WAV_HEADER header;
    uint8_t* data;
    int32_t data_length;
    int32_t samples;
}WAVFile;

bool parseWAVFile(const char* filename, WAVFile* file);
void print_wav_header(const WAV_HEADER* header);




