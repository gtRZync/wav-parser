#pragma once
typedef struct state__ *state;

typedef struct sound {
    char* file_path;
    state state;
} sound;

sound sound_init(const char* file_path);
void  sound_load(sound* _sound);
void  sound_unload(sound* _sound);
void  play_sound(sound* _sound);