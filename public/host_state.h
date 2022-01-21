#ifndef HOST_STATE_H
#define HOST_STATE_H

// This is the bundle of data that is sent to all dlls or game code.
// If statically linking a full game + engine executable, please still use
// this anyways.

// It'll be cleaner usage than the internal engine interfaces.
#include "graphics_context_api.h"
#include "console_api.h"
#include "system_api.h"
#include "audio_api.h"
#include "input.h"

// rename to engine_exports
struct host_state {
    float elapsed_time;

    // these are provided solely you can use them in a more uniform way
    // or for anything that requires an allocator.
    
    // Which might not be many things, since graphics_context has it's own
    // memory management system, that's just a bump allocator...
    
    // system_api also provides allocation methods so feel free to use those
    // instead of system_allocator. (It just helps when I add things that need custom allocators. System's allocator functions may yet disappear.)
    
    // temporary_allocator is fun though. It's cleared every frame! So use it for filters
    // or other functional programming funisms!
    struct allocator* system_allocator;
    struct allocator* temporary_allocator;

    struct graphics_context_api* graphics;
    struct system_api*           system;
    struct input_api*            input;
    struct console_api*          console;
    struct audio_api*            audio;
};

struct host_api {
    void (*initialize)(struct host_state*, int, char**);
    void (*query_render_preferences)(struct graphics_context_limits*);

    void (*frame)(struct host_state*, float);
    void (*fixed_frame)(struct host_state*, float);

    void (*deinitialize)(struct host_state*);
};

struct host_application_description {
    // optional;
    char* title;

    struct {
        int32_t width;
        int32_t height;

        bool fullscreen;
        bool resizable;
    } window;

    // required;
    struct host_api functions;
};

typedef struct                      host_application_description (*shared_object_get_application_description_function)(void);
struct host_application_description get_application_description(void);
#endif
