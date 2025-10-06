#include "soundplayer.h"

int main(int argc, char const *argv[])
{
    sound snd = sound_init("resources/FlappyBird_Menu.wav");
    sound_load(&snd);
    play_sound(&snd);
    playsound_ui_demo(&snd);
    sound_unload(&snd);
    return 0;
}