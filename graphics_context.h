#ifndef GRAPHICS_CONTEXT_H
#define GRAPHICS_CONTEXT_H

// TODO(jerry):
// I don't know what to do to improve post processing, since that's
// probably one of the worse apis here since it's bare minimum.
// I probably won't be able to use it much, but it's there I suppose.


// TODO(jerry): think about instancing as an optimization at some point for particles
// TODO(jerry):
// particles are usually application/game specific though aren't they?

// So the best option would actually just allow an "instanced" rendering mode?
// But this is a leaky abstraction, since "instancing" isn't a thing in 2D...
// I mean it's probably a good idea to leak the abstraction since it still signifies intent...
// So I can simply include two render paths here for sprites / quads
// The instanced path, which is probably just going to be "bulk" rendering. Just provide your own instance data
// based on the format specified here (which should be exposed in the public API),
/*
  likely:

  struct bulk_quad_instance_data {
        float2   position;
        float2   scale;
        float4   texture_subrectangle;
        uint32_t color;
  };
  
  Which would be twelve bytes... And that would be good.
 */
// and do like
// graphics_context_draw_bulk_quads(bulk_quad_data, bulk_quad_data_count, texture, shader);
// Which... I guess isn't too terrible?

#include "public/graphics_context_api.h"

enum graphics_context_vertex_attribute_type {
    GRAPHICS_CONTEXT_VERTEX_ATTRIBUTE_TYPE_FLOAT,
    GRAPHICS_CONTEXT_VERTEX_ATTRIBUTE_TYPE_DOUBLE,
    GRAPHICS_CONTEXT_VERTEX_ATTRIBUTE_TYPE_INT32,
    GRAPHICS_CONTEXT_VERTEX_ATTRIBUTE_TYPE_INT16,
    GRAPHICS_CONTEXT_VERTEX_ATTRIBUTE_TYPE_INT8,
    GRAPHICS_CONTEXT_VERTEX_ATTRIBUTE_TYPE_UINT32,
    GRAPHICS_CONTEXT_VERTEX_ATTRIBUTE_TYPE_UINT16,
    GRAPHICS_CONTEXT_VERTEX_ATTRIBUTE_TYPE_UINT8,
};
enum graphics_context_vertex_buffer_usage_type {
    GRAPHICS_CONTEXT_VERTEX_BUFFER_USAGE_TYPE_DEFAULT, /* will probably just be static in OpenGL */

    GRAPHICS_CONTEXT_VERTEX_BUFFER_USAGE_TYPE_STATIC,
    GRAPHICS_CONTEXT_VERTEX_BUFFER_USAGE_TYPE_DYNAMIC,
};
// Vertex layout abstraction.
struct graphics_context_vertex_attribute {
    char*   name;               // read-only string.
    size_t  element_count;
    size_t  offset;
    bool    normalized;
    uint8_t type;

};
#define GRAPHICS_CONTEXT_VERTEX_FORMAT_MAX_ATTRIBUTE_COUNT (16)
struct graphics_context_vertex_format {
    // count is implicit by how many non full zero members.
    struct graphics_context_vertex_attribute attributes[GRAPHICS_CONTEXT_VERTEX_FORMAT_MAX_ATTRIBUTE_COUNT];
};
enum graphics_context_index_buffer_index_type {
    GRAPHICS_CONTEXT_INDEX_BUFFER_INDEX_TYPE_UINT8,
    GRAPHICS_CONTEXT_INDEX_BUFFER_INDEX_TYPE_UINT16,
    GRAPHICS_CONTEXT_INDEX_BUFFER_INDEX_TYPE_UINT32,
};
struct graphics_context_vertex_buffer_description {
    size_t                                vertex_size;
    size_t                                buffer_count; // this is sized in ELEMENTS, not BYTES.
    // size_t buffer_length = buffer_count * vertex_size;
    void*                                 buffer_data;
    struct graphics_context_vertex_format format;
    uint8_t                               usage;

    struct {
        uint8_t index_type;
        size_t  buffer_count;
        void*   buffer_data;
        uint8_t usage;
    } index_buffer;
};
struct graphics_context_vertex_buffer {
    // This is for book-keeping and debugging I guess, but you don't technically
    // have a reason to store this.
    struct graphics_context_vertex_buffer_description description;
    GLuint                                            index_buffer_object;
    GLuint                                            vertex_buffer_object;
    GLuint                                            vertex_array_object;
};

struct graphics_context_font_glyph {
    uint32_t codepoint;
    graphics_context_texture_handle texture_handle;

    int16_t advance_width;
    int16_t left_side_bearing;

    int16_t bitmap_left;
    int16_t bitmap_top;
    int16_t bitmap_right;
    int16_t bitmap_bottom;

    // Use this to calculate the proper UVs.
    uint16_t _power_of_two_size;

    // NOTE(jerry):
    // only ASCII codepoints are cached like this!
    struct {
        uint16_t texture_coordinate_x;
        uint16_t texture_coordinate_y;
        uint16_t texture_coordinate_w;
        uint16_t texture_coordinate_h;
    };
};
struct graphics_context_font {
    uint8_t status; // resource_status
    char file_path[BLACKIRON_MAX_PLATFORM_PATHNAME_LENGTH];

    // This is to reduce the performance hit from
    // the spare font cache. Rasterizing is slow yes, but I presume a massive chunk of the
    // hits come from disk IO, and not rasterizing. Considering I rasterize text every frame for stupid-slideshow
    // even with unicode, this is probably the case.
    // These file buffers exist on the fixed memory for now...
    // if I find font buffers get excessively big, I might consider dynamically allocating them.
    // (> 8MB or something? TBD)
    // TODO(jerry): store length?
    void* file_buffer;

    float font_size;

    // NOTE(jerry):
    // not sure what to use these metrics for
    // since font_size seems to look correct enough.
    int16_t ascent;
    int16_t descent;
    int16_t line_gap;

    // This is the ascii font cache
    struct graphics_context_font_glyph glyphs[128];
};

struct graphics_context_texture {
    uint8_t status; // resource_status

    char file_path[BLACKIRON_MAX_PLATFORM_PATHNAME_LENGTH];

    // For now we only force "screen" render targets
    // so these are automatically resized all the time.
    bool is_render_target;

    GLuint framebuffer_handle;
    GLuint texture_handle;
    uint32_t width;
    uint32_t height;

    uint32_t _power_of_two_size;
};

struct graphics_context_batch_quad {
    // sprites should not really have access to uniforms at the moment!
    // If they do! This is either a really unique case or a bad idea, and we could
    // either just pack it into the vertex data itself... Or start using instancing?
    float x;
    float y;
    float w;
    float h;

    float uv_x;
    float uv_y;
    // if uv_w and uv_h are 0, assume it is the whole texture being sampled.
    float uv_w;
    float uv_h;

    // This is specified in percentages
    // [0.0 - 1.0]. The only sane way to do pivots!
    float rotation_pivot_x;
    float rotation_pivot_y;
    float rotation_degrees;

    uint8_t flags;
    graphics_context_shader_handle shader;

    uint32_t rgba;
};

// This vertex format is interleaved because I find it to be the easiest way to do it.
struct graphics_context_default_vertex_format {
    float x;
    float y;

    int16_t texture_coordinate_u;
    int16_t texture_coordinate_v;

    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
};

// should this cache uniform locations?
// NOTE(jerry):
// As the graphics context is actually really a renderer and not a true
// dumb context as the it sounds like, the shaders have to follow the vertex format
// that the framework/engine lays out. This would be fine if I made a mini shader DSL
// (have a prelude for all shaders to normalize them).
// But I don't do that so you have to remember to do this:
/*
    VERTEX
    "layout(location=0) in vec2 vertex_attribute_position;\n"
    "layout(location=1) in vec2 vertex_attribute_texcoord;\n"
    "layout(location=2) in vec4 vertex_attribute_color;\n"
    "out vec2 vertex_position;\n"
    "out vec2 vertex_texcoord;\n"
    "out vec4 vertex_color;\n"
    "uniform mat4 view_matrix;\n"
    "uniform mat4 projection_matrix;\n"

    FRAGMENT
    "in vec2 vertex_position;\n"
    "in vec2 vertex_texcoord;\n"
    "in vec4 vertex_color;\n"
    
    in order to have a working shader. this needs to be fixed obviously.
 */
struct graphics_context_shader {
    uint8_t status; // resource_status

    bool from_file;
    char vertex_file_path[BLACKIRON_MAX_PLATFORM_PATHNAME_LENGTH];
    char fragment_file_path[BLACKIRON_MAX_PLATFORM_PATHNAME_LENGTH];

    GLuint program;
};

// This is extra space allocated in the
// batch_vertices for the immediate debug shapes
// lines and circles or N-gons.
#define SPACE_FOR_GENERIC_SHAPE_VERTICES (10000)

struct graphics_context {
    float elapsed_time;

    size_t memory_capacity;
    // double-ended linear allocator.
    size_t memory_used;
    size_t memory_used_top;
    void*  memory;

    uint16_t texture_capacity;
    uint16_t shader_capacity;
    uint16_t font_capacity;
    uint16_t spare_glyph_cache_capacity;
    uint16_t quad_batch_capacity;

    uint16_t immediate_vertices_count;
    uint16_t immediate_indices_count;

    uint16_t shader_count;
    uint16_t texture_count;
    uint16_t font_count;

    int32_t screen_width;
    int32_t screen_height;

    struct {
        float left;
        float right;
        float top;
        float bottom;
    } screen_bounds;

    struct {
        float width;
        float height;
    } virtual_resolution;

    // Hashmaps
    // just using fnv1a since it's the easiest one I know of off the top
    // of my head, and also seems to not be terrible

    // TODO(jerry): Make all of these arrays follow the [1...index-1] range! Only shaders obey this rule right now!

    struct graphics_context_shader*  shaders; //
                                              // NOTE(jerry):
                                              // shaders are not hashtabled!
                                              // this is because I allow loading from either source provided or
                                              // a file! Which means hashing this... Is a bit ambiguous (at least to me)
                                              // since filepaths are just read-only strings (char*) same as source.
                                              //
    struct graphics_context_texture* textures;
    // hybrid hashtable/array. We do hash slots, but also use it for actual array usage.
    // perhaps it is simpler to just treat it as a normal array to reduce complexity from hash
    // table slots. Since the O(1) lookup only needs happen at load time. Or very rarely anyways.
    // We can always store the string hash itself if we're concerned about string comparison I suppose.

    struct graphics_context_font*    fonts;
    // same consideration as the above.

    uint16_t*                           glyph_owners; // hash this along with codepoint to determine if I should recache this with a different glyph
    struct graphics_context_font_glyph* spare_glyph_cache;

    // this should be current batch state
    uint16_t quad_batch_count;

    // TODO(jerry): support multi-textures for some reason. Although... This can get monsterous
    // for batching... So maybe hold off on that? Who knows?
    struct {
        graphics_context_texture_handle texture;
        graphics_context_shader_handle  shader;
    } current_batch;
    graphics_context_texture_handle currently_bound_render_target;

    struct {
        float projection[16];
        float view[16];
    } matrices;

    struct graphics_context_default_vertex_format* batch_vertices;
    uint16_t*                                      batch_indices;

    graphics_context_shader_handle batch_quads_shader_program;
    graphics_context_shader_handle text_shader_program;

    struct graphics_context_vertex_buffer text_vertex_buffer;
    struct graphics_context_vertex_buffer batch_vertex_buffer;

#ifdef DEBUG_BUILD
    uint16_t forward_marker_count;
    uint16_t top_marker_count;
    graphics_context_push_buffer_marker forward_markers[1024];
    graphics_context_push_buffer_marker top_markers[1024];
#endif
};

// NOTE(jerry): the internal graphics_context procedures do not use these because
// the areas we have these are pretty constant so I don't need really need these markers
struct graphics_context_allocation_result graphics_context_push_buffer_allocate_forward(struct graphics_context* graphics_context, size_t amount);
struct graphics_context_allocation_result graphics_context_push_buffer_allocate_top(struct graphics_context* graphics_context, size_t amount);
void                                      graphics_context_push_buffer_restore(struct graphics_context* graphics_context, graphics_context_push_buffer_marker marker);

void graphics_context_initialize(struct graphics_context* graphics_context, struct graphics_context_limits limits);
void graphics_context_deinitialize(struct graphics_context* graphics_context);

// While you can certainly do this yourself... If you really wanted
// but the platform layer takes care of this. So you don't really ever use this.
void graphics_context_set_orthographic_projection(struct graphics_context* graphics_context, float left, float top, float right, float bottom, float near, float far);
void graphics_context_set_viewport(struct graphics_context* graphics_context, float x, float y, float w, float h);

// If either shader arguments are NULL, they will be replaced with
// the default shader source! This is a sane default that allows you to do basic
// sprites... Yay!
graphics_context_shader_handle  graphics_context_load_shader_from_file(struct graphics_context* graphics_context, char* vertex_shader_location, char* fragment_shader_location);
graphics_context_shader_handle  graphics_context_load_shader_from_source(struct graphics_context* graphics_context, char* vertex_shader_source, char* fragment_shader_source);

graphics_context_texture_handle graphics_context_load_texture_from_file(struct graphics_context* graphics_context, char* file_location);
graphics_context_font_handle    graphics_context_load_font_from_file(struct graphics_context* graphics_context, float font_size, char* file_location);

// This is not exposed intentionally. Since I use this for a baked font!
graphics_context_font_handle    graphics_context_load_font_from_buffer_and_keyed_as(struct graphics_context* graphics_context, void* baked_font_file, float font_size, char* key_name);
graphics_context_texture_handle graphics_context_create_render_target_for_screen(struct graphics_context* graphics_context);
struct graphics_context_texture_information graphics_context_texture_information(struct graphics_context* graphics_context, graphics_context_texture_handle texture);

struct graphics_context_font*    graphics_context_dereference_font(struct graphics_context* graphics_context, graphics_context_font_handle font);

// I'm not going to hash, you should be keeping ids
void graphics_context_unload_texture(struct graphics_context* graphics_context, graphics_context_texture_handle texture);
void graphics_context_unload_font(struct graphics_context* graphics_context, graphics_context_font_handle font);
void graphics_context_unload_shader(struct graphics_context* graphics_context, graphics_context_shader_handle shader);

void graphics_context_bind_render_target(struct graphics_context* graphics_context, graphics_context_texture_handle render_target_texture);
void graphics_context_begin_drawing(struct graphics_context* graphics_context, struct camera view);
void graphics_context_end_drawing(struct graphics_context* graphics_context);

void graphics_context_clear_buffer(struct graphics_context* graphics_context, float r, float g, float b, float a);

// sprite batched api
void graphics_context_draw_untextured_quad(struct graphics_context* graphics_context, float x, float y, float w, float h, float r, float g, float b, float a, graphics_context_shader_handle shader);
void graphics_context_draw_untextured_quad_with_rotation(struct graphics_context* graphics_context, float x, float y, float w, float h, float rotation_pivot_x, float rotation_pivot_y, float degrees, float r, float g, float b, float a, graphics_context_shader_handle shader);
void graphics_context_draw_textured_quad(struct graphics_context* graphics_context, graphics_context_texture_handle texture, float x, float y, float w, float h, float r, float g, float b, float a, graphics_context_shader_handle shader);
void graphics_context_draw_textured_quad_with_rotation(struct graphics_context* graphics_context, graphics_context_texture_handle texture, float x, float y, float w, float h, float rotation_pivot_x, float rotation_pivot_y, float degrees, float r, float g, float b, float a, graphics_context_shader_handle shader);
void graphics_context_draw_textured_quad_ext(struct graphics_context* graphics_context, graphics_context_texture_handle texture, float x, float y, float w, float h, float ux, float uy, float uw, float uh, uint8_t flags, float r, float g, float b, float a, graphics_context_shader_handle shader);
void graphics_context_draw_textured_quad_ext_with_rotation(struct graphics_context* graphics_context, graphics_context_texture_handle texture, float x, float y, float w, float h, float ux, float uy, float uw, float uh, uint8_t flags, float rotation_pivot_x, float rotation_pivot_y, float degrees, float r, float g, float b, float a, graphics_context_shader_handle shader);

// NOTE(jerry): This is still at heart a 2D sprite engine
// and not a vector renderer, so vector shapes are second class citizens.
void graphics_context_draw_circle(struct graphics_context* graphics_context, float x, float y, float radius, float r, float g, float b, float a);
void graphics_context_draw_line(struct graphics_context* graphics_context, float x1, float y1, float x2, float y2, float thickness, float r, float g, float b, float a);
// this uses the sprite batcher... So it's draw order isn't the same as the other shapes! Shit!
void graphics_context_draw_rectangle(struct graphics_context* graphics_context, float x, float y, float w, float h, float rotation_degrees, float r, float g, float b, float a);

// text will probably never take a shader... At least not for a while.
void graphics_context_draw_codepoint(struct graphics_context* graphics_context, graphics_context_font_handle font, float x, float y, uint32_t codepoint, float font_scale, float r, float g, float b, float a);
void graphics_context_draw_text(struct graphics_context* graphics_context, graphics_context_font_handle font, float x, float y, char* text_utf8, float font_scale, float r, float g, float b, float a);

void graphics_context_set_virtual_resolution(struct graphics_context* graphics_context, float width, float height);
void graphics_context_map_screenspace_point_into_virtual_resolution(struct graphics_context* graphics_context, float* x, float* y);

void graphics_context_update_shader_uniform(struct graphics_context* graphics_context, graphics_context_shader_handle shader, char* uniform_name, struct graphics_context_shader_uniform_parameter uniform_data);

// scissor test box uses coordinates with the top left as the origin
// instead of bottom left like OpenGL does.
// If the rectangle is set to 0, we will assume that you wanted to reset the
// whole scissor box, since there's no point to having a scissor rectangle of area 0.
// It also accounts for the virtual resolution system.
void graphics_context_set_scissor_region(struct graphics_context* graphics_context, float x, float y, float w, float h);
struct graphics_context_text_extents graphics_context_measure_text(struct graphics_context* graphics_context, graphics_context_font_handle font, char* text_utf8, float font_scale);

// platform layer related helper.
void graphics_context_update_screen_dimensions(struct graphics_context* graphics_context, int32_t new_width, int32_t new_height);
void graphics_context_update(struct graphics_context* graphics_context, float delta_time);


// NOTE(jerry): These are incredibly stupid functions
// that are only here for the initial pass.
// These can be accessed through engine commands
// reload_all_*.
// Although once an automated hotreloading system appears these will be irrelevant.
// They'll stay anyways for the commands incase I have no implemented them on a platform though.

// I am not reloading fonts since those are less likely to change
// also because of the... peculiar requirements they exhibit. This would be very bad for me to try!
// I would have to bite the bullet and cache their file data in a dynamically allocated linked list (to avoid
// using even more memory from a dynamic array).

// Also gratefully. This code is one of the few pieces of code that is not exposed through the public API
// since the user should never have to care about this part!

// Also since these are development only things... I don't care about preserving resources or proper cleanup
// for these, as during release I shouldn't really even be compiling these in. So these will likely leak resources
// to be easier on me.
void graphics_context_reload_all_shaders(struct graphics_context* graphics_context);
void graphics_context_reload_all_textures(struct graphics_context* graphics_context);
void graphics_context_reload_all_fonts(struct graphics_context* graphics_context);

void graphics_context_reload_all_resources(struct graphics_context* graphics_context);

#endif
