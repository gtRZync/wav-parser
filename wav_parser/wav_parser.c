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

#include "wav_parser.h"
#include <errno.h>

void wav_print_header(const wav_header_t* header) {
    printf(COLOR_CYAN "----- WAV Header Info -----\n" COLOR_RESET);
    
    printf(COLOR_GREEN "ChunkID: " COLOR_RESET "%.4s\n", header->RIFF);
    printf(COLOR_GREEN "File Size (minus 8 bytes): " COLOR_RESET "%d bytes\n", header->file_size);
    printf(COLOR_GREEN "Format: " COLOR_RESET "%.4s\n", header->WAVE);
    
    printf("\n" COLOR_YELLOW "Format Subchunk:" COLOR_RESET "\n");
    printf("  " COLOR_BLUE "Subchunk1ID: " COLOR_RESET "%.4s\n", header->fmt);
    printf("  " COLOR_BLUE "Subchunk1 Size: " COLOR_RESET "%d bytes\n", header->chunk_size);
    
    printf("  " COLOR_BLUE "Audio Format: " COLOR_RESET "%d ", header->format_type);
    if (header->format_type == 1)
        printf(COLOR_GREEN "(PCM)\n" COLOR_RESET);
    else
        printf(COLOR_RED "(Compressed/Other)\n" COLOR_RESET);
    
    printf("  " COLOR_BLUE "Number of Channels: " COLOR_RESET "%d ", header->num_channels);
    if (header->num_channels == 1)
        printf(COLOR_GREEN "(Mono)\n" COLOR_RESET);
    else if (header->num_channels == 2)
        printf(COLOR_GREEN "(Stereo)\n" COLOR_RESET);
    else
        printf(COLOR_YELLOW "(Multi-channel)\n" COLOR_RESET);
    
    printf("  " COLOR_BLUE "Sample Rate: " COLOR_RESET "%d Hz\n", header->sample_rate);
    printf("  " COLOR_BLUE "Byte Rate: " COLOR_RESET "%d bytes/sec\n", header->byte_rate);
    printf("  " COLOR_BLUE "Block Align: " COLOR_RESET "%d bytes/frame\n", header->block_align);
    printf("  " COLOR_BLUE "Bits per Sample: " COLOR_RESET "%d bits\n", header->bits_per_sample);

    printf("\n" COLOR_YELLOW "Data Subchunk:" COLOR_RESET "\n");
    printf("  " COLOR_BLUE "Data Header: " COLOR_RESET "%.4s\n", header->data);
    printf("  " COLOR_BLUE "Data Size: " COLOR_RESET "%d bytes\n", header->data_size);

    if (header->byte_rate > 0) {
        double duration_sec = (double)header->data_size / header->byte_rate;
        printf("\n" COLOR_GREEN "Approximate Duration: " COLOR_RESET "%.2f seconds\n", duration_sec);
    }
    
    printf(COLOR_CYAN "---------------------------\n" COLOR_RESET);
}

static bool wav_validate_filename(const char* path) {
    const char* EXTENSION = ".wav";
    
    const char* file = strrchr(path, '/');
    const char* filename = (file == NULL) ? strrchr(path, '\\') : file;
    
    const char* dot = (filename == NULL) ? strrchr(path, '.') : strrchr(filename, '.');
    if(!dot || dot == path) {
        return false;
    }
    
    const char *left = dot - 1;
    if (left >= path && (*left == '\\' || *left == '/')) {
        return false;
    }

    return strcmp(dot, EXTENSION) == 0;
}

static void read_text(char* buff, FILE* file) {
    fread(buff, 4, 1, file);
    buff[4] = '\0';
}

bool wav_parse_file(const char *path, wav_file_t* wav_file)
{
    if(!wav_validate_filename(path)) {
        const char* file = strrchr(path, '/');
        const char* filename = (file == NULL) ? strrchr(path, '\\') + 1 : file + 1;
        fprintf(stderr, COLOR_RED "[ERROR] - Invalid file type." COLOR_BLUE "'%s'" COLOR_RED " is not a valid WAV file. Please provide a .wav file.\n" COLOR_RESET, filename);
        return false;
    }

    FILE* fp;
    bool retval = true;
    fp = fopen(path, "rb");
    if(fp == NULL) {
        fprintf(stderr,COLOR_RED "[ERROR] - Failed to open file : %s\n" COLOR_RESET, path);
        fprintf(stderr, COLOR_YELLOW "[INFO] - Cause : %s.\n" COLOR_RESET, strerror(errno));
        retval = false;
        goto CLOSE_FILE;
    }

    read_text(wav_file->header.RIFF, fp);
    if(strcmp((wav_file->header.RIFF), "RIFF") != 0) {
        fprintf(stderr,COLOR_RED "[ERROR] - %s's first 4 bytes should be \"RIFF\" but are : %s\n" COLOR_RESET, path, wav_file->header.RIFF);
        retval = false;
        goto CLOSE_FILE;
    }

    fread(&wav_file->header.file_size, 4/* bytes */, 1, fp);

    read_text(wav_file->header.WAVE, fp);
    if(strcmp((wav_file->header.WAVE), "WAVE") != 0 ) {
        fprintf(stderr,COLOR_RED "[ERROR] - %s's 4 bytes should be \"WAVE\" but are : %s\n" COLOR_RESET, path, wav_file->header.WAVE);
        retval = false;
        goto CLOSE_FILE;
    }

    read_text(wav_file->header.fmt, fp);
    if(strcmp((wav_file->header.fmt), "fmt ") != 0 ) {
        fprintf(stderr,COLOR_RED "[ERROR] - %s's 4 bytes should be \"fmt/0\" but are : %s\n" COLOR_RESET, path, wav_file->header.fmt);
        retval = false;
        goto CLOSE_FILE;
    }
    fread(&wav_file->header.chunk_size, 4/* bytes */, 1, fp);
    fread(&wav_file->header.format_type, 2/* bytes */, 1, fp);
    if(wav_file->header.format_type != 1) {
        fprintf(stderr,COLOR_RED "[ERROR] - %s's format type should be 1(PCM), but is : %d\n" COLOR_RESET, path, wav_file->header.format_type);
        retval = false;
        goto CLOSE_FILE;
    }

    fread(&wav_file->header.num_channels, 2/* bytes */, 1, fp);
    fread(&wav_file->header.sample_rate, 4/* bytes */, 1, fp);
    fread(&wav_file->header.byte_rate, 4/* bytes */, 1, fp);
    fread(&wav_file->header.block_align, 2/* bytes */, 1, fp);
    fread(&wav_file->header.bits_per_sample, 2/* bytes */, 1, fp);
    if(wav_file->header.bits_per_sample != 16) {
        fprintf(stderr,COLOR_RED "[ERROR] - %s's bits per sample should be 16, but is : %d\n" COLOR_RESET, path, wav_file->header.sample_rate);
        retval = false;
        goto CLOSE_FILE;
    }

    while (fread(wav_file->header.data, 1, 4, fp) == 4) {
        uint32_t chunkSize = 0;
        
        if (fread(&chunkSize, 1, 4, fp) != 4) {
            fprintf(stderr, COLOR_YELLOW "[WARNING] - Unexpected end of file while reading chunk size.\n" COLOR_RESET);
            break;
        }

        if (strncmp(wav_file->header.data, "data", 4) == 0) {
            wav_file->header.data_size = chunkSize;
            break; 
        } else {
            // Skip over this chunk's data
            fseek(fp, chunkSize, SEEK_CUR);
        }
    }
    wav_file->data_length = wav_file->header.data_size;

    wav_file->data = (uint8_t*)malloc(wav_file->data_length);
    if(wav_file->data == NULL) {
        fprintf(stderr,COLOR_RED "[ERROR] - Failed to allocate %d bytes for data.\n" COLOR_RESET, wav_file->data_length);
        fprintf(stderr, COLOR_YELLOW "[INFO] - Cause : %s.\n" COLOR_RESET, strerror(errno));
        retval = false;
        goto CLOSE_FILE;
    }

    if(fread(wav_file->data, 1, wav_file->data_length, fp) != wav_file->data_length) {
        fprintf(stderr,COLOR_RED "[ERROR] - Failed to read data's bytes.\n" COLOR_RESET);
        retval = false;
        goto CLOSE_FILE;
    }
    
    wav_file->samples = wav_file->data_length / wav_file->header.block_align;
    const char* file = strrchr(path, '/');
    const char* filename = (file == NULL) ? strrchr(path, '\\') + 1 : file + 1;
    fprintf(stdout, COLOR_GREEN "\n[INFO] - %s parsed successfully!!!!\n\n" COLOR_RESET, filename);
CLOSE_FILE:
    fclose(fp);
    return retval;
}

void wav_init_file(wav_file_t* wav_file) {
    if(wav_file) {
        memset(wav_file, 0, sizeof(*wav_file));
        memset(&wav_file->header, 0, sizeof(wav_header_t));
    }
}

void wav_free_file(wav_file_t *wav_file)
{
    if (!wav_file) return;

    if (wav_file->data != NULL) {
        free(wav_file->data);
        wav_file->data = NULL; 
        fprintf(stdout, COLOR_GREEN "\n[INFO] - Data section successfully freed!\n\n" COLOR_RESET);
    } else {
        fprintf(stdout, COLOR_YELLOW "\n[WARNING] - No free needed - Data block was not allocated.\n\n" COLOR_RESET);
        return;
    }
    wav_file->data_length = 0;
    wav_file->samples = 0;
    memset(&wav_file->header, 0, sizeof(wav_header_t));
}

