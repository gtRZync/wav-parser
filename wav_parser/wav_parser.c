#include "wav_parser.h"
#include <errno.h>

void wav_print_header(const wav_header_t* header) {
    printf(CYAN "----- WAV Header Info -----\n" RESET);
    
    printf(GREEN "ChunkID: " RESET "%.4s\n", header->RIFF);
    printf(GREEN "File Size (minus 8 bytes): " RESET "%d bytes\n", header->file_size);
    printf(GREEN "Format: " RESET "%.4s\n", header->WAVE);
    
    printf("\n" YELLOW "Format Subchunk:" RESET "\n");
    printf("  " BLUE "Subchunk1ID: " RESET "%.4s\n", header->fmt);
    printf("  " BLUE "Subchunk1 Size: " RESET "%d bytes\n", header->chunk_size);
    
    printf("  " BLUE "Audio Format: " RESET "%d ", header->format_type);
    if (header->format_type == 1)
        printf(GREEN "(PCM)\n" RESET);
    else
        printf(RED "(Compressed/Other)\n" RESET);
    
    printf("  " BLUE "Number of Channels: " RESET "%d ", header->num_channels);
    if (header->num_channels == 1)
        printf(GREEN "(Mono)\n" RESET);
    else if (header->num_channels == 2)
        printf(GREEN "(Stereo)\n" RESET);
    else
        printf(YELLOW "(Multi-channel)\n" RESET);
    
    printf("  " BLUE "Sample Rate: " RESET "%d Hz\n", header->sample_rate);
    printf("  " BLUE "Byte Rate: " RESET "%d bytes/sec\n", header->byte_rate);
    printf("  " BLUE "Block Align: " RESET "%d bytes/frame\n", header->block_align);
    printf("  " BLUE "Bits per Sample: " RESET "%d bits\n", header->bits_per_sample);

    printf("\n" YELLOW "Data Subchunk:" RESET "\n");
    printf("  " BLUE "Data Header: " RESET "%.4s\n", header->data);
    printf("  " BLUE "Data Size: " RESET "%d bytes\n", header->data_size);

    if (header->byte_rate > 0) {
        double duration_sec = (double)header->data_size / header->byte_rate;
        printf("\n" GREEN "Approximate Duration: " RESET "%.2f seconds\n", duration_sec);
    }
    
    printf(CYAN "---------------------------\n" RESET);
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
    printf("left: %s\n", left);

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
        fprintf(stderr, RED "[ERROR] - Invalid file type." BLUE "'%s'" RED " is not a valid WAV file. Please provide a .wav file.\n" RESET, filename);
        return false;
    }

    FILE* fp;
    bool retval = true;
    fp = fopen(path, "rb");
    if(fp == NULL) {
        fprintf(stderr,RED "[ERROR] - Failed to open file : %s\n" RESET, path);
        fprintf(stderr, YELLOW "[INFO] - Cause : %s.\n" RESET, strerror(errno));
        retval = false;
        goto CLOSE_FILE;
    }

    read_text(wav_file->header.RIFF, fp);
    if(strcmp((wav_file->header.RIFF), "RIFF") != 0) {
        fprintf(stderr,RED "[ERROR] - %s's first 4 bytes should be \"RIFF\" but are : %s\n" RESET, path, wav_file->header.RIFF);
        retval = false;
        goto CLOSE_FILE;
    }

    fread(&wav_file->header.file_size, 4/* bytes */, 1, fp);

    read_text(wav_file->header.WAVE, fp);
    if(strcmp((wav_file->header.WAVE), "WAVE") != 0 ) {
        fprintf(stderr,RED "[ERROR] - %s's 4 bytes should be \"WAVE\" but are : %s\n" RESET, path, wav_file->header.WAVE);
        retval = false;
        goto CLOSE_FILE;
    }

    read_text(wav_file->header.fmt, fp);
    if(strcmp((wav_file->header.fmt), "fmt ") != 0 ) {
        fprintf(stderr,RED "[ERROR] - %s's 4 bytes should be \"fmt/0\" but are : %s\n" RESET, path, wav_file->header.fmt);
        retval = false;
        goto CLOSE_FILE;
    }
    fread(&wav_file->header.chunk_size, 4/* bytes */, 1, fp);
    fread(&wav_file->header.format_type, 2/* bytes */, 1, fp);
    if(wav_file->header.format_type != 1) {
        fprintf(stderr,RED "[ERROR] - %s's format type should be 1, but is : %d\n" RESET, path, wav_file->header.format_type);
        retval = false;
        goto CLOSE_FILE;
    }

    fread(&wav_file->header.num_channels, 2/* bytes */, 1, fp);
    fread(&wav_file->header.sample_rate, 4/* bytes */, 1, fp);
    fread(&wav_file->header.byte_rate, 4/* bytes */, 1, fp);
    fread(&wav_file->header.block_align, 2/* bytes */, 1, fp);
    fread(&wav_file->header.bits_per_sample, 2/* bytes */, 1, fp);
    if(wav_file->header.bits_per_sample != 16) {
        fprintf(stderr,RED "[ERROR] - %s's bits per sample should be 16, but is : %d\n" RESET, path, wav_file->header.sample_rate);
        retval = false;
        goto CLOSE_FILE;
    }

    while (fread(wav_file->header.data, 1, 4, fp) == 4) {
        int32_t chunkSize = 0;
        
        if (fread(&chunkSize, 1, 4, fp) != 4) {
            fprintf(stderr, YELLOW "[WARNING] - Unexpected end of file while reading chunk size.\n" RESET);
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
        fprintf(stderr,RED "[ERROR] - Failed to allocate %d bytes for data.\n" RESET, wav_file->data_length);
        fprintf(stderr, YELLOW "[INFO] - Cause : %s.\n" RESET, strerror(errno));
        retval = false;
        goto CLOSE_FILE;
    }

    if(fread(wav_file->data, 1, wav_file->data_length, fp) != wav_file->data_length) {
        fprintf(stderr,RED "[ERROR] - Failed to read data's bytes.\n" RESET);
        retval = false;
        goto CLOSE_FILE;
    }
    
    wav_file->samples = wav_file->data_length / wav_file->header.block_align;
    const char* file = strrchr(path, '/');
    const char* filename = (file == NULL) ? strrchr(path, '\\') + 1 : file + 1;
    fprintf(stdout, GREEN "\n[INFO] - %s parsed successfully!!!!\n\n" RESET, filename);
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
        fprintf(stdout, GREEN "\n[INFO] - Data section successfully freed!\n\n" RESET);
    } else {
        fprintf(stdout, YELLOW "\n[WARNING] - No free needed - Data block was not allocated.\n\n" RESET);
        return;
    }
    wav_file->data_length = 0;
    wav_file->samples = 0;
    memset(&wav_file->header, 0, sizeof(wav_header_t));
}

