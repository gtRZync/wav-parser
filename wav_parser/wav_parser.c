#include "wav_parser.h"
#include <errno.h>

void print_wav_header(const WAV_HEADER* header) {
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



static void read_text(char* buff, FILE* file) {
    fread(buff, 4, 1, file);
    buff[4] = '\0';
}

bool parseWAVFile(const char *filename, WAVFile* wavFile)
{
    bool retval = false;
    FILE* fp;

    fp = fopen(filename, "rb");
    if(fp == NULL) {
        fprintf(stderr,RED "[ERROR] - Failed to open file : %s\n" RESET, filename);
        fprintf(stderr, YELLOW "[INFO] - Cause : %s.\n" RESET, strerror(errno));
        retval = false;
        goto CLOSE_FILE;
    }

    read_text(wavFile->header.RIFF, fp);
    if(strcmp((wavFile->header.RIFF), "RIFF") != 0) {
        fprintf(stderr,RED "[ERROR] - %s's first 4 bytes should be \"RIFF\" but are : %s\n" RESET, filename, wavFile->header.RIFF);
        retval = false;
        goto CLOSE_FILE;
    }

    fread(&wavFile->header.file_size, 4/* bytes */, 1, fp);

    read_text(wavFile->header.WAVE, fp);
    if(strcmp((wavFile->header.WAVE), "WAVE") != 0 ) {
        fprintf(stderr,RED "[ERROR] - %s's 4 bytes should be \"WAVE\" but are : %s\n" RESET, filename, wavFile->header.WAVE);
        retval = false;
        goto CLOSE_FILE;
    }

    read_text(wavFile->header.fmt, fp);
    if(strcmp((wavFile->header.fmt), "fmt ") != 0 ) {
        fprintf(stderr,RED "[ERROR] - %s's 4 bytes should be \"fmt/0\" but are : %s\n" RESET, filename, wavFile->header.fmt);
        retval = false;
        goto CLOSE_FILE;
    }
    fread(&wavFile->header.chunk_size, 4/* bytes */, 1, fp);
    fread(&wavFile->header.format_type, 2/* bytes */, 1, fp);
    if(wavFile->header.format_type != 1) {
        fprintf(stderr,RED "[ERROR] - %s's format type should be 1, but is : %d\n" RESET, filename, wavFile->header.format_type);
        retval = false;
        goto CLOSE_FILE;
    }

    fread(&wavFile->header.num_channels, 2/* bytes */, 1, fp);
    fread(&wavFile->header.sample_rate, 4/* bytes */, 1, fp);
    fread(&wavFile->header.byte_rate, 4/* bytes */, 1, fp);
    fread(&wavFile->header.block_align, 2/* bytes */, 1, fp);
    fread(&wavFile->header.bits_per_sample, 2/* bytes */, 1, fp);
    if(wavFile->header.bits_per_sample != 16) {
        fprintf(stderr,RED "[ERROR] - %s's bits per sample should be 16, but is : %d\n" RESET, filename, wavFile->header.sample_rate);
        retval = false;
        goto CLOSE_FILE;
    }

    //TODO: loop to avoid section that aren't data
    while (fread(wavFile->header.data, 1, 4, fp) == 4) {
        int32_t chunkSize = 0;
        
        if (fread(&chunkSize, 1, 4, fp) != 4) {
            fprintf(stderr, "Unexpected end of file while reading chunk size.\n");
            break;
        }

        if (strncmp(wavFile->header.data, "data", 4) == 0) {
            wavFile->header.data_size = chunkSize;
            break;  // found the data chunk
        } else {
            // Skip over this chunk's data
            fseek(fp, chunkSize, SEEK_CUR);
        }
    }
    wavFile->data_length = wavFile->header.data_size;

    wavFile->data = (uint8_t*)malloc(wavFile->data_length);
    if(wavFile->data == NULL) {
        fprintf(stderr,RED "[ERROR] - Failed to allocate %d bytes for data.\n" RESET, wavFile->data_length);
        fprintf(stderr, YELLOW "[INFO] - Cause : %s.\n" RESET, strerror(errno));
        retval = false;
        goto CLOSE_FILE;
    }

    if(fread(wavFile->data, 1, wavFile->data_length, fp) != wavFile->data_length) {
        fprintf(stderr,RED "[ERROR] - Failed to read data's bytes.\n" RESET, filename, wavFile->header.sample_rate);
        retval = false;
        goto CLOSE_FILE;
    }
    
    wavFile->samples = wavFile->data_length / wavFile->header.block_align;
    fprintf(stdout, GREEN "[INFO] - %s parsed successfully!!!!\n" RESET, filename);
CLOSE_FILE:
    fclose(fp);
    return retval;
}