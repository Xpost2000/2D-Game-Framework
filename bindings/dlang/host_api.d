// This is the giant blackiron import module I guess.
// For dlang. This houses ALL APIs that you could ever want for fun.
module host_api;
import std.stdint;

struct hello_world_limits {
    uint64_t memory_capacity;

    uint16_t textures;
    uint16_t fonts;

    uint16_t glyph_cache;
    uint16_t batch_quads;
}

enum hello_world_flip_bitmask {
    NONE       = 0,
    HORIZONTAL = 1,
    VERTICAL   = 1 << 2,
    BOTH       = HORIZONTAL | VERTICAL
}
enum hello_world_resource_status {
    UNLOADED,
    LOADING,
    READY,
}
enum hello_world_push_buffer_region_type {
    FORWARD,
    TOP,
}
struct hello_world_push_buffer_marker {
    uint8_t region_type;
    size_t marker;
}
struct hello_world_text_extents {
    float width;
    float height;
}
struct hello_world_allocation_result {
    void* memory;
    hello_world_push_buffer_marker restoration_marker;
}

struct hello_world_font_handle {uint16_t id;}
struct hello_world_texture_handle {uint16_t id;}
struct hello_world_shader_handle {uint16_t id;}

__gshared hello_world_font_handle    null_font    = {0};
__gshared hello_world_texture_handle null_texture = {0};
__gshared hello_world_shader_handle  null_shader  = {0};

struct camera {
    float x = 0;
    float y = 0;

    float scale_x = 1;
    float scale_y = 1;

    float rotation = 0;
};

struct hello_world_screen_dimensions {
    int32_t width; 
    int32_t height; 
};

struct hello_world_virtual_dimensions {
    float width; 
    float height; 
};

const BLACKIRON_MAX_PLATFORM_PATHNAME_LENGTH = 64;
struct hello_world_texture_information {
    char[BLACKIRON_MAX_PLATFORM_PATHNAME_LENGTH] file_path;

    uint32_t width;
    uint32_t height;
};


// forward declaration of type.
struct hello_world;

// To be very consistent, if an API is provided from a DLL,
// it's extremely likely these are singleton-like objects, and so I can
// simply pretend they are "global"

// This should simplify client facing code... Massively.
struct hello_world_api {
    extern(C) hello_world_allocation_result function(size_t) push_buffer_allocate_forward;
    extern(C) hello_world_allocation_result function()       push_buffer_allocate_top;
    extern(C) void function(hello_world_push_buffer_marker)  push_buffer_restore;

    extern(C) hello_world_shader_handle  function(const(char*), const(char*)) load_shader_from_source;
    extern(C) hello_world_shader_handle  function(const(char*), const(char*)) load_shader_from_file;
    extern(C) hello_world_texture_handle function(const(char*))        load_texture_from_file;
    extern(C) hello_world_font_handle    function(const(char*), float) load_font_from_file;

    extern(C) void function(hello_world_texture_handle)      unload_texture;
    extern(C) void function(hello_world_font_handle)         unload_font;

    extern(C) void function(camera=camera()) begin_drawing;
    extern(C) void function()                end_drawing;

    extern(C) void function(float, float, float, float) set_scissor_region;
    extern(C) void function(float = 0, float = 0, float = 0, float = 1) clear_buffer;

    extern(C) hello_world_screen_dimensions  function() screen_dimensions;
    extern(C) hello_world_virtual_dimensions function() virtual_dimensions;

    extern(C) void function(float, float, float, float, float, float, float, float, hello_world_shader_handle=null_shader) draw_untextured_quad;
    extern(C) void function(hello_world_texture_handle, float, float, float, float, float, float, float, float, hello_world_shader_handle=null_shader) draw_textured_quad;
    extern(C) void function(hello_world_texture_handle, float, float, float, float, float, float, float, float, uint8_t, float, float, float, float, hello_world_shader_handle=null_shader) draw_textured_quad_ext;

    // I can't understate how fucking cool it is that I can do this
    // function pointers that can take default arguments?
    // I CAN DO C STYLE PIMPL ALL I WANT WITHOUT ANY ISSUES
    // WOOHOO!!!
    extern(C) void function(float = 0, float = 0) set_virtual_resolution;
    extern(C) void function(float* x, float* y)        map_screenspace_point_into_virtual_resolution;

    extern(C) void function(hello_world_font_handle, const(char*), float, float, float, float, float, float, float) draw_text;
    extern(C) void function(hello_world_font_handle, uint32_t, float, float, float, float, float, float, float) draw_codepoint;

    extern(C) hello_world_texture_information function(hello_world_texture_handle texture)  texture_information;
    extern(C) hello_world_text_extents function(hello_world_font_handle, const(char*), float=1.0)      measure_text;

    void with_drawing(void function() inner, camera c=camera()) {
        begin_drawing(c); scope(exit) end_drawing();
        inner();
    }
};

struct host_state {
    float elapsed_time    = 0.0;

    void* system_allocator;
    void* temporary_allocator;

    hello_world_api* graphics;
    void* system;
    void* input;
    void* console;
}

struct host_api {
    extern(C) void function(host_state*, int, char**) initialize                = function(host_state*, int, char**){};
    extern(C) void function(hello_world_limits*) query_render_preferences  = function(hello_world_limits*){};
    extern(C) void function(host_state*, float)       frame                     = function(host_state*, float){};
    extern(C) void function(host_state*)              deinitialize              = function(host_state*){};
}
