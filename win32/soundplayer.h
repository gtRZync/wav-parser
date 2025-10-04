#pragma once
typedef wav_file_t wav;

typedef struct sound {
    char* filename;
    wav wav_file;
} sound;

sound sound_init(const char* filename);
void  sound_load(sound* _sound);
void  sound_unload(sound* _sound);