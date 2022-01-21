#ifndef CONFIGURATION_H
#define CONFIGURATION_H

// Constants will be placed here for more
// These are generally configuration things though.
// convenient editting, if they are often editted.

// Use for most file path strings
// LMAO I had to bump the limit.
#define BLACKIRON_MAX_PLATFORM_PATHNAME_LENGTH (128)

// TODO(jerry): not done.
// #define BLACKIRON_ALLOW_HOTRELOADING_GAME 

// TODO(jerry): These don't do anything yet! Except for static executable.
// That one actually works right now, on windows. Since I haven't split platform
// common code yet.
/* #define BLACKIRON_ALLOW_HOTRELOADING_GAME    0 */
#define BLACKIRON_DEFAULT_GAME_DLL_NAME      "game.dll"

// Platform Layer code
#define BLACKIRON_OPENGL_MAJOR_VERSION 3
#define BLACKIRON_OPENGL_MINOR_VERSION 3
/* #define BLACKIRON_OPENGL_COMPATIBILITY */

// Windows Specific
#define APPLICATION_CONTROLLER_POLL_TIME 0.25
#define MAX_ARGUMENT_VALUE_STRING_POOL_SIZE (4096)
#define MAX_ARGUMENT_VALUE_STRINGS          (128)

struct renderer_limits {
    uint32_t sprites;
    // ensure this is a pot
    uint16_t animations;
    uint16_t lights;
};

struct graphics_context_limits {
    uint64_t memory_capacity;

    // NOTE(jerry):
    // I really don't think anyone needs more than this amount of textures.
    // if it gets bad, I'll bump it to 32 bit. (I may have to promote it to 32 bit anyways
    // for stronger handles.)
    uint16_t textures;
    uint16_t fonts;

    uint16_t glyph_cache;
    uint16_t batch_quads;
};

// Graphics_Context / Graphics
static struct graphics_context_limits graphics_context_default_limits = (struct graphics_context_limits){
    // Recall that fonts also cache their font file within this
    // so keep that in mind!
    // Huge fonts will cause the graphics_context to swallow much more memory!
    .memory_capacity = Mebibyte(32),

    .textures    = 512,
    .fonts       = 128,

    // Ideally this isn't supposed to be so high
    // however the hashing method I use appears to work
    // with the least collisions here. (I can rerasterize and cache 5000 foreign glyphs,
    // which is like a paragraphs worth, above 60fps... Which is pretty ass but I'm paying for it
    // by maintaining a relatively neutral memory footprint (I don't allocate anything new
    // outside whatever the Graphics API may do for me.))
    .glyph_cache = 8192,

    // for the spritebatcher.
    .batch_quads = 8192,
};

// TODO(jerry): make this work.
struct blackiron_application_info {
    char*   name;
    int32_t window_width;
    int32_t window_height;
    bool    fullscreen;
    bool    resizable;
};

#endif
