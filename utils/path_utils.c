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

#include "path_utils.h"
#include <stdbool.h>

static char* clean_path(const char* path) {
    if (!path) return NULL;

    size_t len = strlen(path);
    char* result = malloc(len + 1);  
    if (!result) return NULL;

    size_t j = 0;
    bool last_was_slash = false;

    for (size_t i = 0; i < len; ++i) {
        char c = path[i];

        // Normalize backslash to forward slash
        if (c == '\\') c = '/';

        if (c == '/') {
            if (!last_was_slash) {
                result[j++] = '/';
                last_was_slash = true;
            }
            // else skip duplicate slash
        } else {
            result[j++] = c;
            last_was_slash = false;
        }
    }

    result[j] = '\0';
    return result;
}

char* get_filename(const char* path) {
    if(!path) return NULL;
    char* normalized = clean_path(path);
    const char* slash = strrchr(normalized, '/');
    if(!slash)  slash  = strrchr(normalized, '\\');
    
    const char* filename = (slash) ? slash + 1 : NULL;
    
    if(filename) {
        char* name = (char*)malloc(strlen(filename) + 1);
        if(!name) {
            perror("malloc failed");
            exit(EXIT_FAILURE);
        }
        strcpy(name, filename);
        free(normalized);
        return name;
    } 
    return normalized;
}