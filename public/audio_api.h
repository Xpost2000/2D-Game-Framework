#ifndef AUDIO_API_H
#define AUDIO_API_H

define_new_handle_type(uint16_t, audio_sound_handle);
define_new_handle_type(uint16_t, audio_source_handle);

// TODO(jerry):
// Sources received from play_sound are technically "temporary"
// and expire once they finish playing!

// This is bad.

// I really should just allow you to allocate source and assign it to a
// chunk and let you play them

// then have a separate temporary_sources which comes from play_sound
// Then make play_source a separate thing.

// play_sound would still return a handle to play with, but it might be
// it's supposed to be localized is the idea...

struct audio_api {
    audio_sound_handle  (*load_sound_from_file)(char*);
    void                (*unload_sound)(audio_sound_handle);
    audio_source_handle (*play_sound)(audio_sound_handle, float);
    void                (*set_source_volume)(audio_source_handle, float);
    void                (*stop_all_sounds)(void);
    void                (*set_global_volume)(float);
};

#endif
