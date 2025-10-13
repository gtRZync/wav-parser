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
#include "log.h"

void Log(LogType type, const char* format, ...) {
    va_list args;
    switch(type) {
        case LOG_INFO:      printf(COLOR_GREEN  "[INFO] - ");    break;
        case LOG_WARNING:   printf(COLOR_YELLOW "[WARNING] - "); break;
        case LOG_ERROR:     printf(COLOR_RED    "[ERROR] - ");   break;
    }
    
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
    printf(COLOR_RESET "\n");
}
