#include "soundplayer.h"
#include "wav_parser.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>

#define HIDE_CURSOR     "\033[?25l"
#define SHOW_CURSOR     "\033[?25h"

struct state__ {
    wav_file_t wav_file;
    WAVEFORMATEX format;
    HWAVEOUT hWaveOut;
    WAVEHDR waveHeader;
    bool is_playing;
};

static void WAVEFORMATEX_HDRinit(const sound* snd) {
    wav_file_t* wav_file = &snd->state->wav_file;
    WAVEFORMATEX* format = &snd->state->format;
    WAVEHDR *wvHeader = &snd->state->waveHeader;
    //---------------Format Section-------------------
    format->wFormatTag = WAVE_FORMAT_PCM;
    format->nChannels = wav_file->header.num_channels;
    format->nSamplesPerSec = wav_file->header.sample_rate;
    format->nAvgBytesPerSec = (wav_file->header.sample_rate * wav_file->header.block_align);
    format->nBlockAlign = wav_file->header.block_align;
    format->wBitsPerSample = wav_file->header.bits_per_sample;
    format->cbSize = 0; 
    //---------------Header Section-------------------
    wvHeader->lpData = snd->state->wav_file.data;
    wvHeader->dwBufferLength = snd->state->wav_file.data_length; 
}

static void prepareSoundData(const sound* snd) {
    waveOutOpen(&snd->state->hWaveOut, WAVE_MAPPER, &snd->state->format, (DWORD_PTR)0, (DWORD_PTR)0, CALLBACK_NULL);
    waveOutPrepareHeader(snd->state->hWaveOut, &snd->state->waveHeader, sizeof(snd->state->waveHeader));
}

sound sound_init(const char* file_path) {
    state s = (state)malloc(sizeof(struct state__));
    memset(s, 0, sizeof(struct state__));
    return (sound) {.file_path=file_path, .state=s};
}

void sound_load(sound *snd)
{  
    if(!wav_parse_file(snd->file_path, &snd->state->wav_file)) {
        exit(EXIT_FAILURE);
    }
    WAVEFORMATEX_HDRinit(snd);
    prepareSoundData(snd);
}

void sound_unload(sound *snd)
{
    if (!snd) return;
    if (snd->state != NULL) {
        waveOutUnprepareHeader(snd->state->hWaveOut, &snd->state->waveHeader, sizeof(snd->state->waveHeader));
        waveOutClose(snd->state->hWaveOut);
        free(snd->state);
        snd->state = NULL; 
        fprintf(stdout, COLOR_GREEN "\n[INFO] - Sound state successfully unloaded!\n\n" COLOR_RESET);
    } else {
        fprintf(stdout, COLOR_YELLOW "\n[WARNING] - No free needed - Sound state was not allocated.\n\n" COLOR_RESET);
        return;
    }
    snd->file_path = "";
}

void play_sound(sound *snd)
{
    if(!snd->state->is_playing) {
        waveOutWrite(snd->state->hWaveOut, &snd->state->waveHeader, sizeof(snd->state->waveHeader));
        snd->state->is_playing = true;
    }
}

void sleep(unsigned int milliseconds)
{
    Sleep(milliseconds);
}

// Enable ANSI escape sequences and UTF-8 output on Windows
static void setup_console() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD dwMode = 0;
    if (hOut == INVALID_HANDLE_VALUE) return;
    if (!GetConsoleMode(hOut, &dwMode)) return;
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
    SetConsoleOutputCP(CP_UTF8);
}

static void clear_screen() {
    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hStdOut == INVALID_HANDLE_VALUE) return;

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD count;
    DWORD cellCount;
    COORD homeCoords = {0, 0};

    if (!GetConsoleScreenBufferInfo(hStdOut, &csbi)) return;

    cellCount = csbi.dwSize.X * csbi.dwSize.Y;

    FillConsoleOutputCharacter(hStdOut, (TCHAR) ' ', cellCount, homeCoords, &count);
    FillConsoleOutputAttribute(hStdOut, csbi.wAttributes, cellCount, homeCoords, &count);
    SetConsoleCursorPosition(hStdOut, homeCoords);
}

void playsound_ui_demo(sound* snd) {
    setup_console();

    const char* states[] = { "", ".", "..", "...", "...." };
    size_t size = sizeof(states) / sizeof(states[0]);
    int idx = 0;
    bool *is_playing = &snd->state->is_playing;
    const char* file = strrchr(snd->file_path, '/');
    const char* filename = (file == NULL) ? strrchr(snd->file_path, '\\') + 1 : file + 1; //!will fail if it's not a path

    printf(HIDE_CURSOR);

    while (is_playing) {
        if (snd->state->waveHeader.dwFlags & WHDR_DONE) is_playing = false;

        clear_screen();

        printf(COLOR_CYAN "╔══════════════════════════════════╗\n");
        printf("║ " COLOR_MAGENTA "        🎵 Now Playing   " COLOR_RESET COLOR_CYAN "        ║\n");
        printf("╚══════════════════════════════════╝\n");

        printf(COLOR_GREEN "[File] " COLOR_RESET "%s%s\n\n", filename, states[idx % size]);
        fflush(stdout);
        ++idx;
        sleep(1000);
    }

    printf(COLOR_CYAN "╔══════════════════════════════════╗\n");
    printf("║ " COLOR_MAGENTA "       ✅ Playback Complete      " COLOR_RESET COLOR_CYAN "║\n");
    printf("╚══════════════════════════════════╝\n\n");

    printf(COLOR_GREEN "Thank you for listening! 🎵\n" COLOR_RESET);

    printf(SHOW_CURSOR);
}

