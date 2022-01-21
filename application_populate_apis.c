// NOTE(jerry):
// This file contains the wrappers for all
// apis that are to be exported to the game dll

// The engine api is more verbose than the game version which is what this
// is. Unfortunately generating these wrappers is kind of annoying, but it takes like
// 10 minutes to handroll these, so I'm not too annoyed about it.

// I know we already include them, but explicitly put them here anyways.

// Post Note(jerry):
// Of course, the writing of this, is making me wonder... If I think the game should use a version of these
// with singletons... Why can't the core engine?

// The answer is probably is I should probably stop being so dogmatic about parameters,
// some things really are global, and obviously in this instance I've determined I do want these to be basically global objects.
// Basically any "system" or API that's here... Is likely to be global, so I'll have to handle that. Joy!
#include "public/graphics_context_api.h"
#include "public/system_api.h"
#include "public/input_api.h"
#include "public/console_api.h"
#include "public/audio_api.h"

struct input_api            _global_input_api;
struct graphics_context_api _global_graphics_context_api;
struct system_api           _global_system_api;
struct console_api          _global_console_api;
struct audio_api            _global_audio_api;

struct graphics_context_allocation_result   __graphics_push_buffer_allocate_forward(size_t amount) {return graphics_context_push_buffer_allocate_forward(&application_graphics_context, amount);}
struct graphics_context_allocation_result   __graphics_push_buffer_allocate_top(size_t amount) {return graphics_context_push_buffer_allocate_top(&application_graphics_context, amount);}
void                                        __graphics_push_buffer_restore(graphics_context_push_buffer_marker marker) {graphics_context_push_buffer_restore(&application_graphics_context, marker);}
graphics_context_texture_handle             __graphics_load_texture_from_file(char* filepath) {return graphics_context_load_texture_from_file(&application_graphics_context, filepath);}
graphics_context_texture_handle             __graphics_create_render_target_for_screen(void) {return graphics_context_create_render_target_for_screen(&application_graphics_context);}
graphics_context_font_handle                __graphics_load_font_from_file(char* filepath, float font_size) {return graphics_context_load_font_from_file(&application_graphics_context, font_size, filepath);}
graphics_context_shader_handle              __graphics_load_shader_from_file(char* vertex_file_source, char* fragment_file_source) {return graphics_context_load_shader_from_file(&application_graphics_context, vertex_file_source, fragment_file_source);}
graphics_context_shader_handle              __graphics_load_shader_from_source(char* vertex_source, char* fragment_source) {return graphics_context_load_shader_from_source(&application_graphics_context, vertex_source, fragment_source);}
void                                        __graphics_unload_texture(graphics_context_texture_handle texture){graphics_context_unload_texture(&application_graphics_context, texture);}
void                                        __graphics_unload_font(graphics_context_font_handle font){graphics_context_unload_font(&application_graphics_context, font);}
void                                        __graphics_unload_shader(graphics_context_shader_handle shader){graphics_context_unload_shader(&application_graphics_context, shader);}
void                                        __graphics_bind_render_target(graphics_context_texture_handle render_target_texture){graphics_context_bind_render_target(&application_graphics_context, render_target_texture);}
void                                        __graphics_begin_drawing(struct camera camera){graphics_context_begin_drawing(&application_graphics_context, camera);}
void                                        __graphics_end_drawing(void){graphics_context_end_drawing(&application_graphics_context);}
void                                        __graphics_clear_buffer(float r, float g, float b, float a){graphics_context_clear_buffer(&application_graphics_context, r, g, b, a);}
void                                        __graphics_set_scissor_region(float x, float y, float w, float h){graphics_context_set_scissor_region(&application_graphics_context, x, y, w, h);}
void                                        __graphics_draw_untextured_quad(float x, float y, float w, float h, float r, float g, float b, float a, graphics_context_shader_handle shader){graphics_context_draw_untextured_quad(&application_graphics_context, x, y, w, h, r, g, b, a, shader);}
void                                        __graphics_draw_untextured_quad_with_rotation(float x, float y, float w, float h, float rotation_pivot_x, float rotation_pivot_y, float rotation_degrees, float r, float g, float b, float a, graphics_context_shader_handle shader){graphics_context_draw_untextured_quad_with_rotation(&application_graphics_context, x, y, w, h, rotation_pivot_x, rotation_pivot_y, rotation_degrees, r, g, b, a, shader);}
void                                        __graphics_draw_textured_quad(graphics_context_texture_handle texture, float x, float y, float w, float h, float r, float g, float b, float a, graphics_context_shader_handle shader){graphics_context_draw_textured_quad(&application_graphics_context, texture, x, y, w, h, r, g, b, a, shader);}
void                                        __graphics_draw_textured_quad_with_rotation(graphics_context_texture_handle texture, float x, float y, float w, float h, float rotation_pivot_x, float rotation_pivot_y, float rotation_degrees, float r, float g, float b, float a, graphics_context_shader_handle shader){graphics_context_draw_textured_quad_with_rotation(&application_graphics_context, texture, x, y, w, h, rotation_pivot_x, rotation_pivot_y, rotation_degrees, r, g, b, a, shader);}
void                                        __graphics_draw_textured_quad_ext(graphics_context_texture_handle texture, float x, float y, float w, float h, float uvx, float uvy, float uvw, float uvh, uint8_t flags, float r, float g, float b, float a, graphics_context_shader_handle shader){graphics_context_draw_textured_quad_ext(&application_graphics_context, texture, x, y, w, h, uvx, uvy, uvw, uvh, flags, r, g, b, a, shader);}
void                                        __graphics_draw_textured_quad_ext_with_rotation(graphics_context_texture_handle texture, float x, float y, float w, float h, float uvx, float uvy, float uvw, float uvh, uint8_t flags, float rotation_pivot_x, float rotation_pivot_y, float rotation_degrees, float r, float g, float b, float a, graphics_context_shader_handle shader){graphics_context_draw_textured_quad_ext_with_rotation(&application_graphics_context, texture, x, y, w, h, uvx, uvy, uvw, uvh, flags, rotation_pivot_x, rotation_pivot_y, rotation_degrees, r, g, b, a, shader);}
void                                        __graphics_draw_text(graphics_context_font_handle font, char* utf8_text, float x, float y, float font_scale, float r, float g, float b, float a){graphics_context_draw_text(&application_graphics_context, font, x, y, utf8_text, font_scale, r, g, b, a);}
void                                        __graphics_draw_codepoint(graphics_context_font_handle font, uint32_t codepoint, float x, float y, float font_scale, float r, float g, float b, float a){graphics_context_draw_codepoint(&application_graphics_context, font, x, y, codepoint, font_scale, r, g, b, a);}
void                                        __graphics_update_shader_uniform(graphics_context_shader_handle shader, char* uniform_name, struct graphics_context_shader_uniform_parameter uniform_data) { graphics_context_update_shader_uniform(&application_graphics_context, shader, uniform_name, uniform_data); }
struct graphics_context_text_extents        __graphics_measure_text(graphics_context_font_handle font, char* utf8_text, float font_scale){ return graphics_context_measure_text(&application_graphics_context, font, utf8_text, font_scale);}
struct graphics_context_screen_dimensions   __graphics_get_screen_dimensions(void){
    return (struct graphics_context_screen_dimensions){
        .width = application_graphics_context.screen_width,
        .height = application_graphics_context.screen_height
    };
}
struct graphics_context_virtual_dimensions  __graphics_get_virtual_dimensions(void){
    if (application_graphics_context.virtual_resolution.width == 0 && application_graphics_context.virtual_resolution.height == 0) {
        return (struct graphics_context_virtual_dimensions) {
            .width = application_graphics_context.screen_width,
            .height = application_graphics_context.screen_height,
        };
    } else {
        return (struct graphics_context_virtual_dimensions){
            .width = application_graphics_context.virtual_resolution.width,
            .height = application_graphics_context.virtual_resolution.height
        };
    }
}
void                                        __graphics_map_screenspace_point_into_virtual_resolution(float* x, float* y){graphics_context_map_screenspace_point_into_virtual_resolution(&application_graphics_context, x, y);}
void                                        __graphics_set_virtual_resolution(float width, float height){graphics_context_set_virtual_resolution(&application_graphics_context, width, height);}
struct graphics_context_texture_information __graphics_texture_information(graphics_context_texture_handle texture){return graphics_context_texture_information(&application_graphics_context, texture);}
static void graphics_api_populate(void) {
    _global_graphics_context_api      = (struct graphics_context_api) {
        .push_buffer_allocate_forward = __graphics_push_buffer_allocate_forward,
        .push_buffer_allocate_top     = __graphics_push_buffer_allocate_top,
        .push_buffer_restore          = __graphics_push_buffer_restore,

        .load_shader_from_source         = __graphics_load_shader_from_source,
        .load_shader_from_file           = __graphics_load_shader_from_file,
        .load_texture_from_file          = __graphics_load_texture_from_file,
        .create_render_target_for_screen = __graphics_create_render_target_for_screen,
        .load_font_from_file             = __graphics_load_font_from_file,

        .update_shader_uniform = __graphics_update_shader_uniform,

        .unload_texture = __graphics_unload_texture,
        .unload_font    = __graphics_unload_font,
        .unload_shader  = __graphics_unload_shader,

        .bind_render_target = __graphics_bind_render_target,
        .begin_drawing      = __graphics_begin_drawing,
        .end_drawing        = __graphics_end_drawing,

        .clear_buffer       = __graphics_clear_buffer,
        .set_scissor_region = __graphics_set_scissor_region,

        .draw_untextured_quad   = __graphics_draw_untextured_quad,
        .draw_textured_quad     = __graphics_draw_textured_quad,
        .draw_textured_quad_ext = __graphics_draw_textured_quad_ext,

        .draw_untextured_quad_with_rotation   = __graphics_draw_untextured_quad_with_rotation,
        .draw_textured_quad_with_rotation     = __graphics_draw_textured_quad_with_rotation,
        .draw_textured_quad_ext_with_rotation = __graphics_draw_textured_quad_ext_with_rotation,

        .draw_text          = __graphics_draw_text,
        .draw_codepoint     = __graphics_draw_codepoint,
        .screen_dimensions  = __graphics_get_screen_dimensions,
        .virtual_dimensions = __graphics_get_virtual_dimensions,

        .map_screenspace_point_into_virtual_resolution = __graphics_map_screenspace_point_into_virtual_resolution,
        .set_virtual_resolution                        = __graphics_set_virtual_resolution,
        
        .texture_information = __graphics_texture_information,
        .measure_text        = __graphics_measure_text,
    };
}

static void audio_api_populate(void) {
    /* unimplemented("This audio code is null for now!"); */
    _global_audio_api = (struct audio_api) {
        .load_sound_from_file = audio_load_sound_from_file,
        .unload_sound         = audio_unload_sound,
        .play_sound           = audio_play_sound,
        .stop_all_sounds      = audio_stop_all_sounds,
        .set_global_volume    = audio_set_global_volume,
        .set_source_volume    = audio_set_source_volume,
    };
}

bool                               __input_is_valid_controller(uint8_t controller_index) {return input_controller_valid_controller(&application_input, controller_index);}
bool                               __input_is_controller_button_down(uint8_t controller_index, int32_t button) {return input_is_controller_button_down(&application_input, controller_index, button);}
bool                               __input_is_controller_button_pressed(uint8_t controller_index, int32_t button) {return input_is_controller_button_pressed(&application_input, controller_index, button);}
struct controller_thumbstick_state __input_controller_left_thumbstick(uint8_t controller_index) {return input_controller_left_thumbstick(&application_input, controller_index);}
struct controller_thumbstick_state __input_controller_right_thumbstick(uint8_t controller_index) {return input_controller_right_thumbstick(&application_input, controller_index);}
uint8_t                            __input_controller_left_trigger_pressure(uint8_t controller_index){return input_controller_left_trigger_pressure(&application_input, controller_index);}
uint8_t                            __input_controller_right_trigger_pressure(uint8_t controller_index){return input_controller_right_trigger_pressure(&application_input, controller_index);}
bool                               __input_is_key_down(int32_t key){return input_is_key_down(&application_input, key);}
bool                               __input_is_key_pressed(int32_t key){return input_is_key_pressed(&application_input, key);}
int32_t                            __input_mouse_x(void){return input_mouse_x(&application_input);}
int32_t                            __input_mouse_y(void){return input_mouse_y(&application_input);}
static void input_api_populate(void) {
    _global_input_api = (struct input_api) {
        .valid_controller          = __input_is_valid_controller,
        .controller_button_down    = __input_is_controller_button_down,
        .controller_button_pressed = __input_is_controller_button_pressed,

        .controller_left_thumbstick  = __input_controller_left_thumbstick,
        .controller_right_thumbstick = __input_controller_right_thumbstick,

        .controller_left_trigger_pressure  = __input_controller_left_trigger_pressure,
        .controller_right_trigger_pressure = __input_controller_right_trigger_pressure,

        .key_down    = __input_is_key_down,
        .key_pressed = __input_is_key_pressed,

        .mouse_x = __input_mouse_x,
        .mouse_y = __input_mouse_y,
    };
}

static void console_api_populate(void) {
    _global_console_api = (struct console_api) {
        .register_variable = console_system_register_variable,
        .register_command  = console_system_register_command,

        .find_variable     = console_system_find_variable,
        .active            = console_active,
        .printf            = console_printf,
        .clear             = console_clear,
    };
}

static void system_api_populate(void) {
    // function prototypes declared here
    // to avoid weird declaration formatting in platform files...
    void platform_set_window_title(char*);
    void platform_set_window_resolution(int32_t, int32_t);
    void platform_set_window_resizable(bool);
    void platform_set_window_fullscreen(bool);

    _global_system_api = (struct system_api) {
        .memory_allocate        = system_memory_allocate,
        .memory_deallocate      = system_memory_deallocate,
        .memory_reallocate      = system_memory_reallocate,
        .memory_allocated_total = memory_allocated_total,

        .set_fixed_framerate_update = _set_fixed_framerate_update,

        .read_timestamp_counter = read_timestamp_counter,
        .average_frametime      = framerate_sampler_average_frametime,
        .average_framerate      = framerate_sampler_average_framerate,

        .set_window_title      = platform_set_window_title,
        .set_window_resolution = platform_set_window_resolution,
        .set_window_resizable  = platform_set_window_resizable,
        .set_window_fullscreen = platform_set_window_fullscreen,

        .file_exists           = file_exists,
        .file_size             = get_file_size,
        .read_file_into_buffer = read_file_into_buffer,
        .read_entire_file      = read_entire_file,
        .free_file_buffer      = free_entire_file,
    };
}

void* temporary_allocator_allocate(struct allocator* allocator, size_t amount) {
    struct temporary_allocator* temporary = (struct temporary_allocator*)(allocator);
    void* unaligned_address = temporary->temporary_frame_memory + temporary->used;
    memset(unaligned_address, 0, amount);
    temporary->used += amount;
    return unaligned_address;
}

static void populate_all_apis(struct host_state* state) {
    graphics_api_populate();
    input_api_populate();
    audio_api_populate();
    console_api_populate();
    system_api_populate();

    // setup pointers, and our bundle of joy is ready!
    state->system   = &_global_system_api;
    state->graphics = &_global_graphics_context_api;
    state->input    = &_global_input_api;
    state->console  = &_global_console_api;
    state->audio    = &_global_audio_api;

    // setup allocators
    {
        _global_temporary_allocator.interface.allocate   = temporary_allocator_allocate;
        // The bump allocator doesn't store any information, so reallocate isn't possible
        // because to simulate behavior I'd have to be able to copy between regions!
        _global_temporary_allocator.interface.reallocate = allocator_reallocate_null;
        _global_temporary_allocator.interface.free       = allocator_free_null;
    }

    // TODO(jerry): system allocator.
    {
        
    }

    state->temporary_allocator = &_global_temporary_allocator.interface;
    state->system_allocator    = &system_allocator;
}
