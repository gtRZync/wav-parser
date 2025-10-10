#include "soundplayer.h"
#include "wav_parser.h"
#include <windows.h>
#define HIDE_CURSOR     "\033[?25l"
#define SHOW_CURSOR     "\033[?25h"

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

static void playsound_ui_demo(sound* snd) {
    setup_console();

    const char* states[] = { "", ".", "..", "...", "...." };
    size_t size = sizeof(states) / sizeof(states[0]);
    int idx = 0;
    const char* file = strrchr(snd->file_path, '/');
    const char* filename = (file == NULL) ? strrchr(snd->file_path, '\\') + 1 : file + 1; //!will fail if it's not a path

    printf(HIDE_CURSOR);

    while (is_playing(snd)) {

        clear_screen();

        printf(COLOR_CYAN "╔══════════════════════════════════╗\n");
        printf("║ " COLOR_MAGENTA "        🎵 Now Playing   " COLOR_RESET COLOR_CYAN "        ║\n");
        printf("╚══════════════════════════════════╝\n");

        printf(COLOR_GREEN "[File] " COLOR_RESET "%s%s\n\n", filename, states[idx % size]);
        fflush(stdout);
        ++idx;
        Sleep(1000);
    }

    printf(COLOR_CYAN "╔══════════════════════════════════╗\n");
    printf("║ " COLOR_MAGENTA "       ✅ Playback Complete      " COLOR_RESET COLOR_CYAN "║\n");
    printf("╚══════════════════════════════════╝\n\n");

    printf(COLOR_GREEN "Thank you for listening! 🎵\n" COLOR_RESET);

    printf(SHOW_CURSOR);
}


int main(int argc, char const *argv[])
{
    sound snd = sound_init("resources/FlappyBird_Menu.wav");
    sound_load(&snd);
    play_sound(&snd);
    playsound_ui_demo(&snd);
    sound_unload(&snd);
    return 0;
}