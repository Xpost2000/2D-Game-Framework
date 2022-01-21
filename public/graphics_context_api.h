#ifndef GRAPHICS_CONTEXT_API_H
#define GRAPHICS_CONTEXT_API_H

// Technically... I'm supposed to be making a game,
// but spending a few weeks on the engine part is fine.

// since as is, once I add audio and spend another day on core. I can technically
// make games.

// Anyways, this the public dll exportable API for the graphics context.
// there's a lot of things in here, and it'll have to mirror and match
// whatever's in graphics_context.h

// this is our flags field referred to in textured_quad_ex. Technically these are the only flags but whatever.
enum graphics_context_flip_bitmask {
    GRAPHICS_CONTEXT_FLIP_NONE = 0,
    GRAPHICS_CONTEXT_FLIP_HORIZONTAL = BIT(0),
    GRAPHICS_CONTEXT_FLIP_VERTICAL   = BIT(1),
    GRAPHICS_CONTEXT_FLIP_BOTH       = GRAPHICS_CONTEXT_FLIP_HORIZONTAL | GRAPHICS_CONTEXT_FLIP_VERTICAL
};
// TODO(jerry): make it illegal to have 0 as an id, nothing hashes to 0 right now so it won't really hurt if I don't do it yet.
enum graphics_context_resource_status {
    GRAPHICS_CONTEXT_RESOURCE_STATUS_UNLOADED,
    GRAPHICS_CONTEXT_RESOURCE_STATUS_LOADING, // this is for whenever
                                              // asynchronous IO is done
                                              // for now, this is simply unused.
    GRAPHICS_CONTEXT_RESOURCE_STATUS_READY,
};
/*
  The graphics_context will provide access to it's relatively massive chunks of rendering memory.
  
  Obviously since this is a generic memory allocator, I can't really enforce that this is being
  used strictly for graphics reasons, but try to only use this for graphics arrays.
  
  (IE: Sprite arrays, or lighting calculations or something...)
*/
enum graphics_context_push_buffer_region_type {
    GRAPHICS_CONTEXT_PUSH_BUFFER_REGION_TYPE_FORWARD,
    GRAPHICS_CONTEXT_PUSH_BUFFER_REGION_TYPE_TOP,
};
typedef struct graphics_context_push_buffer_marker {
    uint8_t region_type;
    size_t marker;
} graphics_context_push_buffer_marker;
struct graphics_context_text_extents {
    float width;
    float height;
};
struct graphics_context_allocation_result {
    void* memory;
    graphics_context_push_buffer_marker restoration_marker;
};

define_new_handle_type(uint16_t, graphics_context_font_handle);
define_new_handle_type(uint16_t, graphics_context_texture_handle);

define_new_handle_type(uint16_t, graphics_context_shader_handle);
static graphics_context_font_handle    null_font         = {0};
static graphics_context_texture_handle null_texture      = {0};
static graphics_context_texture_handle null_rendertarget = {0};
static graphics_context_shader_handle  null_shader       = {0};

// use this to provide a camera.
// will set view matrix on begin_drawing when provided
// although you should really keep track of your camera.
// this is just to avoid making the parameter list arbitrarily large
// this thing assumes origin is top-left.
struct camera {
    float x;
    float y;

    float scale_x;
    float scale_y;

    // radians probably.
    float rotation;
};

struct graphics_context_screen_dimensions {
    int32_t width; 
    int32_t height; 
};

struct graphics_context_virtual_dimensions {
    float width; 
    float height; 
};

struct graphics_context_texture_information {
    char file_path[BLACKIRON_MAX_PLATFORM_PATHNAME_LENGTH];

    uint32_t width;
    uint32_t height;
};


// forward declaration of type.
struct graphics_context;

// To be very consistent, if an API is provided from a DLL,
// it's extremely likely these are singleton-like objects, and so I can
// simply pretend they are "global"

enum graphics_context_shader_uniform_parameter_type {
    GRAPHICS_CONTEXT_SHADER_UNIFORM_PARAMETER_TYPE_FLOAT,
    GRAPHICS_CONTEXT_SHADER_UNIFORM_PARAMETER_TYPE_INTEGER,
    GRAPHICS_CONTEXT_SHADER_UNIFORM_PARAMETER_TYPE_MATRIX,
};
struct graphics_context_shader_uniform_parameter {
    enum graphics_context_shader_uniform_parameter_type type;
    size_t count;

    union {
        struct {
            float values[4];
        } floating_point;

        struct {
            int32_t values[4];
        } integer;

        // only allow passing matrix pointers, no copy by value.
        struct {
            float* data;
        } matrix;
    };
};
// This should simplify client facing code... Massively.
struct graphics_context_api {
    struct graphics_context_allocation_result (*push_buffer_allocate_forward)(size_t);
    struct graphics_context_allocation_result (*push_buffer_allocate_top)();
    void (*push_buffer_restore)(graphics_context_push_buffer_marker);

    graphics_context_shader_handle  (*load_shader_from_source)(char*, char*);
    graphics_context_shader_handle  (*load_shader_from_file)(char*, char*);
    graphics_context_texture_handle (*load_texture_from_file)(char*);
    graphics_context_texture_handle (*create_render_target_for_screen)(void);
    graphics_context_font_handle    (*load_font_from_file)(char*, float);
    void (*unload_texture)(graphics_context_texture_handle);
    void (*unload_font)(graphics_context_font_handle);
    void (*unload_shader)(graphics_context_shader_handle);

    void (*update_shader_uniform)(graphics_context_shader_handle, char*, struct graphics_context_shader_uniform_parameter);

    void (*bind_render_target)(graphics_context_texture_handle);
    void (*begin_drawing)(struct camera);
    void (*end_drawing)(void);

    void (*set_scissor_region)(float, float, float, float);
    void (*clear_buffer)(float, float, float, float);

    struct graphics_context_screen_dimensions  (*screen_dimensions)(void);
    struct graphics_context_virtual_dimensions (*virtual_dimensions)(void);

    void (*draw_untextured_quad)(float, float, float, float, float, float, float, float, graphics_context_shader_handle);
    void (*draw_untextured_quad_with_rotation)(float, float, float, float, float, float, float, float, float, float, float, graphics_context_shader_handle);
    void (*draw_textured_quad)(graphics_context_texture_handle, float, float, float, float, float, float, float, float, graphics_context_shader_handle);
    void (*draw_textured_quad_with_rotation)(graphics_context_texture_handle, float, float, float, float, float, float, float, float, float, float, float, graphics_context_shader_handle);
    void (*draw_textured_quad_ext)(graphics_context_texture_handle, float, float, float, float, float, float, float, float, uint8_t, float, float, float, float, graphics_context_shader_handle);
    void (*draw_textured_quad_ext_with_rotation)(graphics_context_texture_handle, float, float, float, float, float, float, float, float, uint8_t, float, float, float, float, float, float, float, graphics_context_shader_handle);

    void (*set_virtual_resolution)(float width, float height);
    // NOTE(jerry):
    // only into virtual resolution. You are responsible for mapping into world space if you have
    // cameras with the graphics context!
    void (*map_screenspace_point_into_virtual_resolution)(float* x, float* y);

    // draw_codepoint, not needed because you usually don't want it.
    // Especially once I have utf8 text... Which still hasn't happened
    void (*draw_text)(graphics_context_font_handle, char*, float, float, float, float, float, float, float);
    void (*draw_codepoint)(graphics_context_font_handle, uint32_t, float, float, float, float, float, float, float);

    // Dereference operators are disabled!
    // TODO(jerry): we need to provide ways of accessing public texture information!
    struct graphics_context_texture_information (*texture_information)(graphics_context_texture_handle texture);
    struct graphics_context_text_extents (*measure_text)(graphics_context_font_handle, char*, float);
};

#endif
