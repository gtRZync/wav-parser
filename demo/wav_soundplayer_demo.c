#include "soundplayer.h"
#include "playsound_ui_demo.h"

int main(int argc, char const *argv[])
{
    sound *snd = sound_init("resources/sound/bass-wiggle.wav");
    play_sound(snd);
    playsound_ui_demo(snd);
    //?for replay demo purposes
    play_sound(snd);
    playsound_ui_demo(snd);
    sound_unload(snd);
    return 0;
}