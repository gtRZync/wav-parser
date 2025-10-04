#include "wav_parser.h"

int main(int argc, char const *argv[])
{
    wav_file_t file;
    wav_init_file(&file);
    wav_parse_file("resources/FlappyBird_Menu.wav", &file);
    wav_print_header(&file.header);
    wav_free_file(&file);
    return 0;
}
