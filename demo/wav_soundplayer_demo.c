#include "soundplayer.h"

int main(int argc, char const *argv[])
{
    sound snd = sound_init("resources/FlappyBird_Menu.wav");
    sound_load(&snd);
    play_sound(&snd);
    sleep(41250);
    sound_unload(&snd);
    return 0;
}
