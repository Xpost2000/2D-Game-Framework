// NOTE(jerry):
// SDL mixer isn't great but it's a quick driver to get results
// as I can't do audio in time for Major Jam, so for now I need this
// as a backend

// To match future behavior, I'm probably only going to allow
// fully loaded audio, and not allow streaming since for smallish games
// this isn't necessary and most of my engines are write-once discard later
// so I'm not going to think too hard about this and just hack it away and
// shelve better ideas away for the future or when I'm really really serious
// about making a game with an engine.

// I know that SDL2 has an actual low level audio device I can use for more
// correct mixing, but this is way faster right now cause I still kind of suck at raw
// audio mixing, and I want this to be an easy day so yeah...

// This is really just gamejam in C material kind of stuff anyways...
#ifndef SDL2
// IE: The plan is to implement external library audio drivers
// but I will implement my own audio driver for this engine. 
#error "Cannot use SDL2 Mixer Audio device without SDL2 build!"
#error "If I'm doing this for audio reasons. There is no audio driver for win32 yet!"
#error "I could use OpenAL or something though."
#endif

#include <SDL2/SDL_mixer.h>

enum sdl2_audio_device_constants {
    SDL2_AUDIO_MAX_AUDIO_SOURCES     = 8192,
    SDL2_AUDIO_MIXER_AUDIO_FREQUENCY = 44100,

    SDL2_AUDIO_MIXER_MIXING_CHUNK_SIZE  = 4096,

    SDL2_AUDIO_MIXER_MIXING_CHUNK_COUNT = 1024,
    SDL2_AUDIO_MAX_LOADED_SOUNDS        = SDL2_AUDIO_MIXER_MIXING_CHUNK_COUNT,
};

struct sdl2_mixer_audio_source {
    audio_sound_handle playing_sound_id;
    uint32_t                 channel_id;
};

struct sdl2_mixer_audio_device {
    uint16_t                       sound_count;
    Mix_Chunk*                     sounds[SDL2_AUDIO_MAX_LOADED_SOUNDS];
    
    uint16_t                       source_count;
    struct sdl2_mixer_audio_source sources[SDL2_AUDIO_MAX_AUDIO_SOURCES];

    bool initialized;
};
static struct sdl2_mixer_audio_device _global_audio_device = {0};

// Need to do this
void audio_open_device(void) {
    if (_global_audio_device.initialized == false) {
        Mix_Init(MIX_INIT_OGG | MIX_INIT_MOD);
        Mix_OpenAudio(SDL2_AUDIO_MIXER_AUDIO_FREQUENCY, AUDIO_S16, 2, SDL2_AUDIO_MIXER_MIXING_CHUNK_SIZE);
        Mix_AllocateChannels(SDL2_AUDIO_MIXER_MIXING_CHUNK_COUNT);
        _global_audio_device.initialized = true;
        console_printf("SDL2_MixerAudioDevice: opened\n");
    }
}

void audio_close_device(void) {
    if (_global_audio_device.initialized == true) {
        console_printf("SDL2_MixerAudioDevice: closed\n");
        Mix_CloseAudio();
        Mix_Quit();
        _global_audio_device.initialized = false;
    }
}

void audio_set_global_volume(float volume) {
    Mix_Volume(-1, (int32_t)(volume * MIX_MAX_VOLUME));
}

audio_sound_handle audio_load_sound_from_file(char* path) {
    console_printf("SDL2_MixerAudioDevice loading sound \"%s\"\n", path);
    audio_sound_handle handle_result = {};

    assertion(_global_audio_device.sound_count < SDL2_AUDIO_MAX_LOADED_SOUNDS);

    uint64_t hash_key            = fnv1a_hash(path, strlen(path));
    uint16_t resource_hash_index = hash_key & (SDL2_AUDIO_MAX_LOADED_SOUNDS - 1);
    handle_result.id = resource_hash_index;

    if (_global_audio_device.sounds[resource_hash_index] == NULL) {
        Mix_Chunk** sound_resource = &_global_audio_device.sounds[resource_hash_index];
        *sound_resource            = Mix_LoadWAV(path);
    }

    _global_audio_device.sound_count++;
    return handle_result;
}

void audio_unload_sound(audio_sound_handle sound) {
    if (_global_audio_device.sounds[sound.id]) {
        Mix_FreeChunk(_global_audio_device.sounds[sound.id]);
        _global_audio_device.sound_count--;
        _global_audio_device.sounds[sound.id] = NULL;
    }
}

audio_source_handle audio_play_sound(audio_sound_handle sound, float volume) {
    if (_global_audio_device.source_count < SDL2_AUDIO_MAX_AUDIO_SOURCES) {
        audio_source_handle handle = { .id = _global_audio_device.source_count++ };

        struct sdl2_mixer_audio_source* current_source = &_global_audio_device.sources[handle.id];
        int32_t channel_id = Mix_PlayChannel(-1, _global_audio_device.sounds[sound.id], 0);

        if (channel_id == -1) {
            _global_audio_device.sound_count--;
        } else {
            Mix_Volume(channel_id, (int32_t)(volume * MIX_MAX_VOLUME));

            current_source->playing_sound_id = sound;
            current_source->channel_id       = channel_id;
        }

        return handle;
    }

    return (audio_source_handle){
        .id = 0
    };
}

void audio_set_source_volume(audio_source_handle source, float volume) {
    struct sdl2_mixer_audio_source* current_source = &_global_audio_device.sources[source.id];
    Mix_Volume(current_source->channel_id, (int32_t)(volume * MIX_MAX_VOLUME));
}

void audio_update(float delta_time) {
    for (size_t audio_source_index = _global_audio_device.sound_count-1; audio_source_index != (size_t)(-1); --audio_source_index) {
        struct sdl2_mixer_audio_source* current_source = &_global_audio_device.sources[audio_source_index];

        // should be correct.
        // not really too plussed about checking.
        if (Mix_Playing(current_source->channel_id) == false) {
            _global_audio_device.sources[audio_source_index] = _global_audio_device.sources[--_global_audio_device.sound_count];
        }
    }
}

void audio_stop_all_sounds(void) {
    Mix_HaltChannel(-1);
}
