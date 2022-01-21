#ifndef AUDIO_H
#define AUDIO_H

#include "public/audio_api.h"

// NOTE(jerry):
//
// For now this is the barest and simplest API that
// would be suitable for a game right now.

// It's not really advanced audio, but I'm not an audio guy :P
// Not really a graphics guy either, but most people can graphics better than audio.

/*
  I'll try to open the most reasonable audio device by default.
  
  For the most part that's probably just going to be 44100hz and unsigned 16 bit audio.
*/

// Audio is not being hot reloaded by the way! This can be dangerous!
// Well. I can always double buffer the audio... Which is fine I guess.
// Or push them into a "commit" buffer, and update it when all sources using the sound
// are finished playing.

// NOTE(jerry): Audio... should happen on it's own thread if possible!
void audio_open_device(void);
void audio_close_device(void);

// 0.0 - 1.0
void audio_set_global_volume(float volume);

audio_sound_handle  audio_load_sound_from_file(char* path);
void                audio_unload_sound(audio_sound_handle sound);

audio_source_handle audio_play_sound(audio_sound_handle sound, float volume);
void                audio_set_source_volume(audio_source_handle source, float volume);

void                audio_pause_source(audio_source_handle source);
void                audio_set_source_volume(audio_source_handle source, float volume);

void audio_update(float delta_time);
void audio_stop_all_sounds(void);

#endif
