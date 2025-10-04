#include "wav_parser/wav_parser.h"

int main(int argc, char const *argv[])
{
    WAVFile file;
    parseWAVFile("resources/FlappyBird_Menu.wav", &file);
    print_wav_header(&file.header);
    free(file.data);
    return 0;
}
