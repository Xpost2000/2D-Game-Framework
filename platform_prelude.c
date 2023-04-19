// This is to maintain inclusion order.
// Because it's a unity build so that's kind of a thing.

// Technically I should be making a build.c or something
// though.

// NOTE(jerry):
// anyways about giving the choice of unity or no unity build...
// I'm going full unity build, and seeing how it feels.
// There is a disadvantage in that inclusion order... Does matter
// but it's a pretty minor price since it seems to build very fast
// and also makes building very easy since you can compile with
// a one liner basically.
// maybe use gl46?
/* #include <bundled_external/glad_generated/gl.h> */

// TODO(jerry):
// GLEW is not bundled, but I already have it setup. I would rather just
// use glad, but I've been unable to setup header-only...
#include <GL/glew.h>
/* #include <bundled_external/glad_generated/gl1/gl.h> */

#include "common.h"
#include "public/utf8.h"
#include "public/utf8.c"

#include "public/configuration.h"

#include "public/vector2d.h"
#include "public/collision.h"
#include "public/word_iterator.h"
#include "public/host_state.h"
#include "public/allocator_interface.h"

#include "input.h"
#include "audio.h"
#include "graphics_context.h"
#include "console.h"
#include "read_entire_file.h"
#include "file_watcher.h"

static bool                    application_quit                        = false;
static struct input            application_input                       = {0};
static struct graphics_context application_graphics_context            = {0};
static float                   application_poll_controller_state_timer = 0;

static struct allocator        system_allocator             = {0};

// The temporary allocator's memory is cleared EVERY frame.
// It's a giant linear allocator that is used for one shot allocations
// great for doing functional programming stuff.
// This should be like 64K so we don't run out of memory when we least
// expect it...
struct temporary_allocator {
    struct allocator interface;

    size_t used;
    char   temporary_frame_memory[Kibibyte(64)];
};
static struct temporary_allocator _global_temporary_allocator = {0};

static float target_framerate         = 0.0f; // no fixed update by default
static float target_inverse_framerate = 0.0;
static float frametime_accumulator    = 0.0;

void _set_fixed_framerate_update(int32_t frames_per_second) {
    target_framerate         = (float)frames_per_second;
    target_inverse_framerate = 1.0f/target_inverse_framerate;
}

#include "application_populate_apis.c"

#include "input.c"
#include "read_entire_file.c"
#include "graphics_context.c"
#include "console.c"

#include "public/minischeme.h"
#include "public/minischeme.c"
#include "public/word_iterator.c"

#include "main_common.c"

// OS Specific drivers 
// for certain engine components!
// Use configuration to handle these for now I guess...

#ifdef _WIN32
#include "file_watcher_win32.c"
#elif defined(__linux__)
#include "file_watcher_linux.c"
#else
#include "file_watcher_null.c"
#endif
#include "file_watcher_common.c"

// TODO(jerry): No win32 backend
#ifdef SDL2
#include "audio_sdl2_mixer.c"
#else
#include "audio_null.c"
#endif

