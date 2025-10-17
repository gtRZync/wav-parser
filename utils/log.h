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
#include <stdarg.h>

#define COLOR_RED       "\033[1;31m"
#define COLOR_GREEN     "\033[1;32m"
#define COLOR_YELLOW    "\033[1;33m"
#define COLOR_BLUE      "\033[1;34m"
#define COLOR_CYAN      "\033[1;36m"
#define COLOR_MAGENTA   "\033[1;35m"
#define COLOR_RESET     "\033[0m"

typedef enum LogType {
    LOG_INFO,
    LOG_WARNING,
    LOG_ERROR
}LogType;

/**
 * Logs a formatted message to the standard output with a colored log level prefix.
 *
 * Supported log levels:
 *   - LOG_INFO    → Green     "[INFO] - "
 *   - LOG_WARNING → Yellow    "[WARNING] - "
 *   - LOG_ERROR   → Red       "[ERROR] - "
 *
 * Example usage:
 *   Log(LOG_INFO, "Loaded file: %s", filename);
 *   Log(LOG_ERROR, "Failed to open device: %d", errorCode);
 *
 * Color codes:
 *   - COLOR_GREEN
 *   - COLOR_YELLOW
 *   - COLOR_RED
 *   - COLOR_RESET (used at the end of the line)
 *
 * @param type   The log level (LOG_INFO, LOG_WARNING, LOG_ERROR).
 * @param format The printf-style format string.
 * @param ...    Additional arguments for formatting.
 *
 * @warning
 *   If your format string includes custom ANSI color codes, do NOT end with COLOR_RESET.
 *   Instead, reuse the log level's color constant at the end of your format to preserve
 *   consistent prefix coloring.
 *
 *   ✅ Good:
 *     Log(LOG_INFO, COLOR_GREEN "Custom green message" COLOR_GREEN);
 *
 *   ❌ Bad:
 *     Log(LOG_INFO, COLOR_GREEN "Message but resets color" COLOR_RESET);
 *
 *   This prevents the prefix color from being broken mid-line or in multiline logs.
 */
void Log(LogType type, const char* format, ...);