// TODO(jerry): Both _push_batch_ functions do not handle proper overflow! This can be bad!

// I trust STB to not be buggy
// NOTE(jerry): There are quite a lot of holes in this implementation... Mostly buffer overruns, which
// I have to checkout later.
#ifdef __GNUC__ 
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic push
#endif

#define STB_IMAGE_IMPLEMENTATION
#include <bundled_external/stb_image.h>

#define STBTT_STATIC
#define STB_TRUETYPE_IMPLEMENTATION
#include <bundled_external/stb_truetype.h>

#ifdef __GNUC__ 
#pragma GCC diagnostic pop
#endif

// prototype for the file hentai.
void _graphics_context_reload_resource(char* file_name, void* user_data);

/////////////////// BEGIN Graphics Abstraction
// This is used to centralize most of the non-significant graphics API code.
// small fry like glViewport, glScissor or one liners are left out, but objects are mostly kept here
// uniforms are also placed here.

// These are redundant but allow me to trace them if I want to.
// These are less efficient since they rebind without caching.
// Hopefully the framerate doesn't get hit.
void _opengl_shader_uniform_set_float1(GLuint shader_program, char* uniform_name, float v0) {
    glUseProgram(shader_program);
    GLuint uniform_location = glGetUniformLocation(shader_program, uniform_name);
    glUniform1f(uniform_location, v0);
}
void _opengl_shader_uniform_set_float2(GLuint shader_program, char* uniform_name, float v0, float v1) {
    glUseProgram(shader_program);
    GLuint uniform_location = glGetUniformLocation(shader_program, uniform_name);
    glUniform2f(uniform_location, v0, v1);
}
void _opengl_shader_uniform_set_float3(GLuint shader_program, char* uniform_name, float v0, float v1, float v2) {
    glUseProgram(shader_program);
    GLuint uniform_location = glGetUniformLocation(shader_program, uniform_name);
    glUniform3f(uniform_location, v0, v1, v2);
}
void _opengl_shader_uniform_set_float4(GLuint shader_program, char* uniform_name, float v0, float v1, float v2, float v3) {
    glUseProgram(shader_program);
    GLuint uniform_location = glGetUniformLocation(shader_program, uniform_name);
    glUniform4f(uniform_location, v0, v1, v2, v3);
}

void _opengl_shader_uniform_set_integer1(GLuint shader_program, char* uniform_name, int v0) {
    glUseProgram(shader_program);
    GLuint uniform_location = glGetUniformLocation(shader_program, uniform_name);
    glUniform1i(uniform_location, v0);
}
void _opengl_shader_uniform_set_integer2(GLuint shader_program, char* uniform_name, int v0, int v1) {
    glUseProgram(shader_program);
    GLuint uniform_location = glGetUniformLocation(shader_program, uniform_name);
    glUniform2i(uniform_location, v0, v1);
}
void _opengl_shader_uniform_set_integer3(GLuint shader_program, char* uniform_name, int v0, int v1, int v2) {
    glUseProgram(shader_program);
    GLuint uniform_location = glGetUniformLocation(shader_program, uniform_name);
    glUniform3i(uniform_location, v0, v1, v2);
}
void _opengl_shader_uniform_set_integer4(GLuint shader_program, char* uniform_name, int v0, int v1, int v2, int v3) {
    glUseProgram(shader_program);
    GLuint uniform_location = glGetUniformLocation(shader_program, uniform_name);
    glUniform4i(uniform_location, v0, v1, v2, v3);
}
void _opengl_shader_uniform_set_matrix4x4(GLuint shader_program, char* uniform_name, size_t count, float* matrix) {
    glUseProgram(shader_program);
    GLuint uniform_location = glGetUniformLocation(shader_program, uniform_name);
    glUniformMatrix4fv(uniform_location, count, GL_FALSE, matrix);
}

#if __EMSCRIPTEN__
static char* DEFAULT_VERTEX_SOURCE =
    "#version 300 es\n"
    "precision highp float;\n"
    "layout(location=0) in vec2 vertex_attribute_position;\n"
    "layout(location=1) in vec2 vertex_attribute_texcoord;\n"
    "layout(location=2) in vec4 vertex_attribute_color;\n"
    "out vec2 vertex_position;\n"
    "out vec2 vertex_texcoord;\n"
    "out vec4 vertex_color;\n"
    "uniform mat4 view_matrix;\n"
    "uniform mat4 projection_matrix;\n"
    "void main(){\n"
    // TODO(jerry): enforce row matrices! Although they are less SIMD friendly, but I don't really care about performance right now.
    //              Semantically, it's easier to understand.
    //                                                     (because of layout reasons...)

    // Personally I use row matrices, but to lessen my work and avoid too much thinking just use column matrix until I get this setup more. (left multiply)
    "gl_Position = projection_matrix * view_matrix * vec4(vertex_attribute_position.x, vertex_attribute_position.y, 0.0, 1.0);\n"
    "vertex_position = vertex_attribute_position;\n"
    "vertex_texcoord = vertex_attribute_texcoord;\n"
    "vertex_color    = vertex_attribute_color;\n"
    "}\n"
    ;

static char* DEFAULT_FRAGMENT_SOURCE =
    "#version 300 es\n"
    "precision highp float;\n"
    "in vec2 vertex_position;\n"
    "in vec2 vertex_texcoord;\n"
    "in vec4 vertex_color;\n"
    "uniform sampler2D sampler_texture;\n"
    "uniform float     using_texture;\n"
    "out vec4 output_color;\n"
    "void main(){\n"
    // branchless shader to do texture sampling.
    "output_color = mix(vertex_color, texture(sampler_texture, vertex_texcoord) * vertex_color, using_texture);\n"
    "}\n"
    ;

static char* TEXT_FRAGMENT_SOURCE =
    "#version 300 es\n"
    "precision highp float;\n"
    "in vec2 vertex_position;\n"
    "in vec2 vertex_texcoord;\n"
    "in vec4 vertex_color;\n"
    "uniform sampler2D sampler_texture;\n"
    "out vec4 output_color;\n"
    "void main(){\n"
    "float text_alpha = texture(sampler_texture, vertex_texcoord).a;\n"
    "output_color = vec4(vertex_color.rgb, text_alpha * vertex_color.a);\n"
    "}\n"
    ;

#else
static char* DEFAULT_VERTEX_SOURCE =
    "#version 330\n"
    "layout(location=0) in vec2 vertex_attribute_position;\n"
    "layout(location=1) in vec2 vertex_attribute_texcoord;\n"
    "layout(location=2) in vec4 vertex_attribute_color;\n"
    "out vec2 vertex_position;\n"
    "out vec2 vertex_texcoord;\n"
    "out vec4 vertex_color;\n"
    "uniform mat4 view_matrix;\n"
    "uniform mat4 projection_matrix;\n"
    "void main(){\n"
    // TODO(jerry): enforce row matrices! Although they are less SIMD friendly, but I don't really care about performance right now.
    //              Semantically, it's easier to understand.
    //                                                     (because of layout reasons...)

    // Personally I use row matrices, but to lessen my work and avoid too much thinking just use column matrix until I get this setup more. (left multiply)
    "gl_Position = projection_matrix * view_matrix * vec4(vertex_attribute_position.x, vertex_attribute_position.y, 0.0, 1.0);\n"
    "vertex_position = vertex_attribute_position;\n"
    "vertex_texcoord = vertex_attribute_texcoord;\n"
    "vertex_color    = vertex_attribute_color;\n"
    "}\n"
    ;

static char* DEFAULT_FRAGMENT_SOURCE =
    "#version 330\n"
    "in vec2 vertex_position;\n"
    "in vec2 vertex_texcoord;\n"
    "in vec4 vertex_color;\n"
    "uniform sampler2D sampler_texture;\n"
    "uniform float     using_texture = 0;"
    "out vec4 output_color;\n"
    "void main(){\n"
    // branchless shader to do texture sampling.
    "output_color = mix(vertex_color, texture(sampler_texture, vertex_texcoord) * vertex_color, using_texture);\n"
    "}\n"
    ;

static char* TEXT_FRAGMENT_SOURCE =
    "#version 330\n"
    "in vec2 vertex_position;\n"
    "in vec2 vertex_texcoord;\n"
    "in vec4 vertex_color;\n"
    "uniform sampler2D sampler_texture;\n"
    "out vec4 output_color;\n"
    "void main(){\n"
    "float text_alpha = texture(sampler_texture, vec2(vertex_texcoord.x, vertex_texcoord.y)).r;\n"
    "output_color = vec4(vertex_color.rgb, text_alpha * vertex_color.a);\n"
    "}\n"
    ;
#endif

// I really shouldn't assume these are zero-terminated C strings
// I swear this is going to bite me in the ass later.
struct graphics_context_shader_description {
    char* vertex_source;
    char* fragment_source;
};
#define GRAPHICS_CONTEXT_MAX_SHADER_LOG_LENGTH 8192
struct graphics_context_shader_result {
    bool   error;
    GLuint program;
    
    char vertex_error_log[GRAPHICS_CONTEXT_MAX_SHADER_LOG_LENGTH];
    char fragment_error_log[GRAPHICS_CONTEXT_MAX_SHADER_LOG_LENGTH];
    char linkage_error_log[GRAPHICS_CONTEXT_MAX_SHADER_LOG_LENGTH];
};
struct graphics_context_shader_result _opengl_create_shader_program(struct graphics_context_shader_description description) {
    struct graphics_context_shader_result result = {};
    
    if (description.vertex_source && description.fragment_source) {
        GLuint shader_program = glCreateProgram();
        GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
        GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);

        {
            glShaderSource(vertex_shader, 1, (const GLchar* const*)&description.vertex_source, 0);
            glCompileShader(vertex_shader);

            GLint compilation_status;
            glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &compilation_status);

            if (compilation_status == GL_FALSE) {
                result.error = true;
                glGetShaderInfoLog(vertex_shader, GRAPHICS_CONTEXT_MAX_SHADER_LOG_LENGTH, 0, result.vertex_error_log);
            }
            
        }

        {
            glShaderSource(fragment_shader, 1, (const GLchar* const*)&description.fragment_source, 0);
            glCompileShader(fragment_shader);

            GLint compilation_status;
            glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &compilation_status);

            if (compilation_status == GL_FALSE) {
                result.error = true;
                glGetShaderInfoLog(fragment_shader, GRAPHICS_CONTEXT_MAX_SHADER_LOG_LENGTH, 0, result.fragment_error_log);
            }
            
        }

        glAttachShader(shader_program, vertex_shader);
        glAttachShader(shader_program, fragment_shader);

        glLinkProgram(shader_program);
        
        {
            GLint link_status;
            glGetProgramiv(shader_program, GL_LINK_STATUS, &link_status);

            if (link_status == GL_FALSE) {
                result.error = true;
                glGetProgramInfoLog(shader_program, GRAPHICS_CONTEXT_MAX_SHADER_LOG_LENGTH, 0, result.linkage_error_log);
            }
        }
        
        glDetachShader(shader_program, vertex_shader);
        glDetachShader(shader_program, fragment_shader);
        
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);

        result.program = shader_program;
    } else {
        result.error = true;
        if (!description.vertex_source) {
            snprintf(result.vertex_error_log, GRAPHICS_CONTEXT_MAX_SHADER_LOG_LENGTH, "no vertex shader provided\n");
        }
        if (!description.fragment_source) {
            snprintf(result.fragment_error_log, GRAPHICS_CONTEXT_MAX_SHADER_LOG_LENGTH, "no fragment shader provided\n");
        }
    }

    return result;
}

enum {
    // according to https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glTexParameter.xhtml
    GRAPHICS_CONTEXT_DEFAULT_MIPMAP_LEVEL = 1000,
};
// set to sane defaults... Mostly.
enum graphics_context_texture_filter_type {
    GRAPHICS_CONTEXT_TEXTURE_FILTER_TYPE_DEFAULT, // will probably be nearest

    GRAPHICS_CONTEXT_TEXTURE_FILTER_TYPE_NEAREST,
    GRAPHICS_CONTEXT_TEXTURE_FILTER_TYPE_LINEAR,
};
GLint _map_native_texture_filter_enum_into_opengl(GLint input) {
    switch (input) {
        case GRAPHICS_CONTEXT_TEXTURE_FILTER_TYPE_NEAREST: return GL_NEAREST;
        case GRAPHICS_CONTEXT_TEXTURE_FILTER_TYPE_LINEAR:  return GL_LINEAR;
    }
    return input;
}
enum graphics_context_texture_wrap_type {
    GRAPHICS_CONTEXT_TEXTURE_WRAP_TYPE_REPEAT,
    GRAPHICS_CONTEXT_TEXTURE_WRAP_TYPE_CLAMP_TO_EDGE,
    GRAPHICS_CONTEXT_TEXTURE_WRAP_TYPE_CLAMP_TO_BORDER,
    GRAPHICS_CONTEXT_TEXTURE_WRAP_TYPE_MIRROR_REPEAT,
    GRAPHICS_CONTEXT_TEXTURE_WRAP_TYPE_MIRROR_CLAMP_TO_EDGE,
};
GLint _map_native_texture_wrap_enum_into_opengl(GLint input) {
    switch (input) {
        case GRAPHICS_CONTEXT_TEXTURE_WRAP_TYPE_REPEAT:               return GL_REPEAT;
        case GRAPHICS_CONTEXT_TEXTURE_WRAP_TYPE_CLAMP_TO_EDGE:        return GL_CLAMP_TO_EDGE;
        case GRAPHICS_CONTEXT_TEXTURE_WRAP_TYPE_CLAMP_TO_BORDER:      return GL_CLAMP_TO_BORDER;
        case GRAPHICS_CONTEXT_TEXTURE_WRAP_TYPE_MIRROR_REPEAT:        return GL_MIRRORED_REPEAT;
        case GRAPHICS_CONTEXT_TEXTURE_WRAP_TYPE_MIRROR_CLAMP_TO_EDGE: return GL_MIRROR_CLAMP_TO_EDGE;
    }
    return input;
}
// directly maps to OpenGL
enum graphics_context_texture_format {
    GRAPHICS_CONTEXT_TEXTURE_FORMAT_RGBA32,
    GRAPHICS_CONTEXT_TEXTURE_FORMAT_RGB24,

    GRAPHICS_CONTEXT_TEXTURE_FORMAT_R8,
    GRAPHICS_CONTEXT_TEXTURE_FORMAT_RG16,

    GRAPHICS_CONTEXT_TEXTURE_FORMAT_BGR24,
    GRAPHICS_CONTEXT_TEXTURE_FORMAT_BGRA32,
};
GLint _map_native_texture_format_enum_into_opengl(GLint input) {
    switch (input) {
        case  GRAPHICS_CONTEXT_TEXTURE_FORMAT_RGBA32: return GL_RGBA;
        case  GRAPHICS_CONTEXT_TEXTURE_FORMAT_RGB24:  return GL_RGB;

#if __EMSCRIPTEN__
        case  GRAPHICS_CONTEXT_TEXTURE_FORMAT_R8:     return GL_ALPHA;
#else
        case  GRAPHICS_CONTEXT_TEXTURE_FORMAT_R8:     return GL_RED;
#endif
        case  GRAPHICS_CONTEXT_TEXTURE_FORMAT_RG16:   return GL_RG;

        case  GRAPHICS_CONTEXT_TEXTURE_FORMAT_BGR24:  return GL_BGR;
        case  GRAPHICS_CONTEXT_TEXTURE_FORMAT_BGRA32: return GL_BGRA;
    };
    return input;
}
struct graphics_context_texture_description {
    GLint base_level;
    GLint mipmap_level;

    GLint min_filter;
    GLint mag_filter;
    
    GLint wrap_t;
    GLint wrap_s;

    GLint internal_format;
    GLint format;

    GLsizei unpack_alignment;
};

#define OPENGL_DEFAULT_UNPACK_ALIGNMENT (4)
// A combined gen textures and glTexParameters with image2D.
GLuint _opengl_create_new_texture2d(struct graphics_context_texture_description description, uint32_t image_width, uint32_t image_height, uint8_t* image_pixels) {
    if (description.min_filter == GRAPHICS_CONTEXT_TEXTURE_FILTER_TYPE_DEFAULT) {
        description.min_filter = GRAPHICS_CONTEXT_TEXTURE_FILTER_TYPE_NEAREST;
    }

    if (description.mag_filter == GRAPHICS_CONTEXT_TEXTURE_FILTER_TYPE_DEFAULT) {
        description.mag_filter = GRAPHICS_CONTEXT_TEXTURE_FILTER_TYPE_NEAREST;
    }

    if (description.unpack_alignment == 0) {
        description.unpack_alignment = OPENGL_DEFAULT_UNPACK_ALIGNMENT;
    }

    GLuint gl_texture_handle;

    glGenTextures(1, &gl_texture_handle);
    glBindTexture(GL_TEXTURE_2D, gl_texture_handle);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, description.base_level);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL,  description.mipmap_level);
    if (description.mipmap_level > 0) {
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, _map_native_texture_filter_enum_into_opengl(description.min_filter));
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, _map_native_texture_filter_enum_into_opengl(description.mag_filter));
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, _map_native_texture_wrap_enum_into_opengl(description.wrap_t));
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, _map_native_texture_wrap_enum_into_opengl(description.wrap_s));

    glPixelStorei(GL_UNPACK_ALIGNMENT, description.unpack_alignment);
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 _map_native_texture_format_enum_into_opengl(description.internal_format),
                 image_width, image_height, 0,
                 _map_native_texture_format_enum_into_opengl(description.format),
                 GL_UNSIGNED_BYTE, image_pixels);
    glBindTexture(GL_TEXTURE_2D, 0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, OPENGL_DEFAULT_UNPACK_ALIGNMENT);

    return gl_texture_handle;
}

GLint _map_native_vertex_attribute_enum_into_opengl(uint8_t input) {
    switch (input) {
        case GRAPHICS_CONTEXT_VERTEX_ATTRIBUTE_TYPE_FLOAT:  return GL_FLOAT;
        case GRAPHICS_CONTEXT_VERTEX_ATTRIBUTE_TYPE_DOUBLE: return GL_DOUBLE;
        case GRAPHICS_CONTEXT_VERTEX_ATTRIBUTE_TYPE_INT32:  return GL_INT;
        case GRAPHICS_CONTEXT_VERTEX_ATTRIBUTE_TYPE_INT16:  return GL_SHORT;
        case GRAPHICS_CONTEXT_VERTEX_ATTRIBUTE_TYPE_INT8:   return GL_BYTE;
        case GRAPHICS_CONTEXT_VERTEX_ATTRIBUTE_TYPE_UINT32: return GL_UNSIGNED_INT;
        case GRAPHICS_CONTEXT_VERTEX_ATTRIBUTE_TYPE_UINT16: return GL_UNSIGNED_SHORT;
        case GRAPHICS_CONTEXT_VERTEX_ATTRIBUTE_TYPE_UINT8:  return GL_UNSIGNED_BYTE;
    }

    return input;
}
size_t _map_index_buffer_index_type_enum_into_element_size(uint8_t input) {
    switch (input) {
        case GRAPHICS_CONTEXT_INDEX_BUFFER_INDEX_TYPE_UINT8:  return 1;
        case GRAPHICS_CONTEXT_INDEX_BUFFER_INDEX_TYPE_UINT16: return 2;
        case GRAPHICS_CONTEXT_INDEX_BUFFER_INDEX_TYPE_UINT32: return 4;
    }

    return 0;
}
GLint _map_index_buffer_index_type_enum_into_opengl(uint8_t input) {
    switch (input) {
        case GRAPHICS_CONTEXT_INDEX_BUFFER_INDEX_TYPE_UINT8:  return GL_UNSIGNED_BYTE;
        case GRAPHICS_CONTEXT_INDEX_BUFFER_INDEX_TYPE_UINT16: return GL_UNSIGNED_SHORT;
        case GRAPHICS_CONTEXT_INDEX_BUFFER_INDEX_TYPE_UINT32: return GL_UNSIGNED_INT;
    }
    return input;
}
GLint _map_native_vertex_buffer_usage_enum_into_opengl(uint8_t input) {
    switch (input) {
        case GRAPHICS_CONTEXT_VERTEX_BUFFER_USAGE_TYPE_STATIC:  return GL_STATIC_DRAW;
        case GRAPHICS_CONTEXT_VERTEX_BUFFER_USAGE_TYPE_DYNAMIC: return GL_DYNAMIC_DRAW;
    }

    return input;
}
struct graphics_context_vertex_buffer _opengl_create_vertex_buffer(struct graphics_context_vertex_buffer_description buffer_description) {
    GLuint vertex_buffer_object = 0;
    GLuint vertex_array_object  = 0;
    GLuint index_buffer_object  = 0;

    glGenBuffers(1, &vertex_buffer_object);
    glGenVertexArrays(1, &vertex_array_object);

    // initialize buffer.
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object);
    {
        size_t buffer_size = buffer_description.vertex_size * buffer_description.buffer_count;

        if (buffer_description.usage == GRAPHICS_CONTEXT_VERTEX_BUFFER_USAGE_TYPE_DEFAULT) {
            buffer_description.usage = GRAPHICS_CONTEXT_VERTEX_BUFFER_USAGE_TYPE_STATIC;
        }
        
        glBufferData(GL_ARRAY_BUFFER, buffer_size, buffer_description.buffer_data, _map_native_vertex_buffer_usage_enum_into_opengl(buffer_description.usage));
    }

    // setup the index buffer.
    // if we should
    {
        // buffer_count != 0 should be sane...
        if (buffer_description.index_buffer.buffer_count > 0) {
            glGenBuffers(1, &index_buffer_object);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer_object);

            size_t element_size = _map_index_buffer_index_type_enum_into_element_size(buffer_description.index_buffer.index_type);
            size_t buffer_size = buffer_description.index_buffer.buffer_count * element_size;

            glBufferData(GL_ELEMENT_ARRAY_BUFFER, buffer_size, buffer_description.index_buffer.buffer_data, _map_native_vertex_buffer_usage_enum_into_opengl(buffer_description.index_buffer.usage));
        }
    }

    // setup opengl vertex attribute state with the vertex array.
    glBindVertexArray(vertex_array_object);
    {
        for (size_t vertex_attribute_index = 0; vertex_attribute_index < GRAPHICS_CONTEXT_VERTEX_FORMAT_MAX_ATTRIBUTE_COUNT; ++vertex_attribute_index) {
            struct graphics_context_vertex_attribute* current_vertex_attribute = &buffer_description.format.attributes[vertex_attribute_index];

            bool empty_attribute = true;
            // Check for any "non-zero" member.
            // Excluding the name which is technically optional here.
            {
                if (empty_attribute && current_vertex_attribute->element_count) {
                    empty_attribute = false;
                } else if (empty_attribute && current_vertex_attribute->offset) {
                    empty_attribute = false;
                } else if (empty_attribute && current_vertex_attribute->type) {
                    empty_attribute = false;
                }
            }

            if (!empty_attribute) {
                glEnableVertexAttribArray(vertex_attribute_index);
                glVertexAttribPointer(vertex_attribute_index,
                                      current_vertex_attribute->element_count,
                                      _map_native_vertex_attribute_enum_into_opengl(current_vertex_attribute->type),
                                      current_vertex_attribute->normalized,
                                      buffer_description.vertex_size,
                                      (void*)current_vertex_attribute->offset);
            }
        }
    }

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    return (struct graphics_context_vertex_buffer) {
        .description          = buffer_description,
        .vertex_array_object  = vertex_array_object,
        .vertex_buffer_object = vertex_buffer_object,
        .index_buffer_object  = index_buffer_object,
    };
}

////////////////////// END of Graphics API Abstractions

struct graphics_context_viewport_rectangle {
    float x;
    float y;
    float w;
    float h;

    float scale_factor;
};
static bool _should_pillarbox(float width, float height, float virtual_width, float virtual_height) {
    float physical_aspect_ratio = width / height;
    float logical_aspect_ratio  = virtual_width / virtual_height;
    return (physical_aspect_ratio > logical_aspect_ratio);
}


// NOTE(jerry):
// I implicitly specced the API according to graphics_context_api to
// have virtual resolution == 0 == real resolution
// but none of this code actually does that so I'm doing the special casing here.
// There's going to be a huge change of stuff anyways, so most of this should be kind of gone.
static float _optimal_aspect_ratio_scale_factor(float width, float height, float virtual_width, float virtual_height) {
    if (_should_pillarbox(width, height, virtual_width, virtual_height)) {
        return height / virtual_height;
    } else {
        return width / virtual_width;
    }
}
static struct graphics_context_viewport_rectangle _viewport_rectangle_for_virtual_resolution(float width, float height, float virtual_width, float virtual_height) {
    if (virtual_height == 0 && virtual_width == 0) {
        virtual_height = height;
        virtual_width  = width;
    }

    struct graphics_context_viewport_rectangle viewport = {};

    // from virtual to real.
    float aspect_ratio_scale_factor = _optimal_aspect_ratio_scale_factor(width, height, virtual_width, virtual_height);

    viewport.w = (virtual_width * aspect_ratio_scale_factor);
    viewport.h = (virtual_height * aspect_ratio_scale_factor);
    viewport.x = (width - viewport.w) / 2.0f;
    viewport.y = (height - viewport.h) / 2.0f;

    viewport.scale_factor = aspect_ratio_scale_factor;

    return viewport;
}

static void _graphics_context_memory_usage_printout(struct graphics_context* graphics_context) {
#if 0
    {
        size_t c = graphics_context->memory_capacity;
        console_printf("%llu B (%llu KB), %llu MB capacity\n", c, c / 1024, c / (1024*1024));
        size_t u = graphics_context->memory_used + graphics_context->memory_used_top;
        if (u > c) {
            size_t o = u - c;
            console_printf("FATAL: You have overrun memory by: %llu B (%llu KB) %llu MB!\n", o, o/1024, o/(1024*1024));
        }
        console_printf("%llu B, %llu KB, %llu MB\n", u, u / 1024, u / (1024*1024));
        size_t l = c - u;
        console_printf("%llu B, %llu KB, %llu MB left\n", l, l / 1024, l / (1024*1024));
    }
#endif
}

static void* _graphics_context_push_buffer_unaligned_alloc(struct graphics_context* graphics_context, size_t amount) {
    void* result = graphics_context->memory + graphics_context->memory_used;
    graphics_context->memory_used += amount;

    size_t total_used = graphics_context->memory_used + graphics_context->memory_used_top;
    // This assertion will fire until we start asking for more memory blocks I suppose.
    _graphics_context_memory_usage_printout(graphics_context);
    assertion(total_used < graphics_context->memory_capacity && "(forward alloc)Run out of graphics_context memory!");
    return result;
}

static void* _graphics_context_push_buffer_unaligned_alloc_top(struct graphics_context* graphics_context, size_t amount) {
    graphics_context->memory_used_top += amount;
    void* result = (graphics_context->memory + graphics_context->memory_capacity) - graphics_context->memory_used_top;

    size_t total_used = graphics_context->memory_used + graphics_context->memory_used_top;
    _graphics_context_memory_usage_printout(graphics_context);
    assertion(total_used < graphics_context->memory_capacity && "(top alloc)Run out of graphics_context memory!");
    return result;
}


static void* _graphics_context_read_file_into_buffer(struct graphics_context* graphics_context, char* file_location) {
    size_t file_buffer_length = get_file_size(file_location);

    void* new_file_buffer = _graphics_context_push_buffer_unaligned_alloc_top(graphics_context, file_buffer_length);
    read_file_into_buffer(file_location, new_file_buffer, file_buffer_length);

    _graphics_context_memory_usage_printout(graphics_context);
    return new_file_buffer;
}

// NOTE(jerry): A macro can easily be used to make this more ergonomic, however I don't do that here.
struct graphics_context_allocation_result graphics_context_push_buffer_allocate_forward(struct graphics_context* graphics_context, size_t amount) {
    size_t marker_location = graphics_context->memory_used;
    void* memory = _graphics_context_push_buffer_unaligned_alloc(graphics_context, amount);

    struct graphics_context_allocation_result result = (struct graphics_context_allocation_result) {
        .memory = memory,
        .restoration_marker = (graphics_context_push_buffer_marker) {
            .region_type = GRAPHICS_CONTEXT_PUSH_BUFFER_REGION_TYPE_FORWARD,
            .marker = marker_location
        }
    };

#ifdef DEBUG_BUILD
    if (amount > 0) {
        graphics_context->top_markers[graphics_context->top_marker_count++] = result.restoration_marker;
    }
#endif
    return result;
}

struct graphics_context_allocation_result graphics_context_push_buffer_allocate_top(struct graphics_context* graphics_context, size_t amount) {
    size_t marker_location = graphics_context->memory_used_top;
    void* memory = _graphics_context_push_buffer_unaligned_alloc_top(graphics_context, amount);

    struct graphics_context_allocation_result result = (struct graphics_context_allocation_result) {
        .memory = memory,
        .restoration_marker = (graphics_context_push_buffer_marker) {
            .region_type = GRAPHICS_CONTEXT_PUSH_BUFFER_REGION_TYPE_TOP,
            .marker = marker_location
        }
    };

#ifdef DEBUG_BUILD
    if (amount > 0) {
        graphics_context->forward_markers[graphics_context->forward_marker_count++] = result.restoration_marker;
    }
#endif
    return result;
}

void graphics_context_push_buffer_restore(struct graphics_context* graphics_context, graphics_context_push_buffer_marker marker) {
    switch (marker.region_type) {
        case GRAPHICS_CONTEXT_PUSH_BUFFER_REGION_TYPE_TOP: {
#ifdef DEBUG_BUILD
            if (graphics_context->top_marker_count > 0) {
                graphics_context_push_buffer_marker* most_recent_marker = &graphics_context->top_markers[graphics_context->top_marker_count-1];
                assert(most_recent_marker->marker == marker.marker && "You are freeing push buffer allocations out of order!");
            }
#endif

            graphics_context->memory_used_top = marker.marker;
        } break;
        case GRAPHICS_CONTEXT_PUSH_BUFFER_REGION_TYPE_FORWARD: {
#ifdef DEBUG_BUILD
            if (graphics_context->forward_marker_count > 0) {
                graphics_context_push_buffer_marker* most_recent_marker = &graphics_context->forward_markers[graphics_context->forward_marker_count-1];
                assert(most_recent_marker->marker == marker.marker && "You are freeing push buffer allocations out of order!");
            }
#endif

            graphics_context->memory_used = marker.marker;
        } break;
    }
}

enum graphics_context_resource_type {
    GRAPHICS_CONTEXT_RESOURCE_TYPE_TEXTURE,
    GRAPHICS_CONTEXT_RESOURCE_TYPE_SHADER,
    /* GRAPHICS_CONTEXT_RESOURCE_TYPE_FONT, */
};
struct graphics_context_file_hentai_user_data_packet {
    struct graphics_context* context;

    uint8_t resource_type;
    union {
        graphics_context_texture_handle texture;
        graphics_context_shader_handle shader;
    };
};

void graphics_context_initialize(struct graphics_context* graphics_context, struct graphics_context_limits limits) {
    graphics_context->memory_capacity = limits.memory_capacity;
    graphics_context->memory_used     = 0;
    graphics_context->memory_used_top = 0;
    graphics_context->memory          = system_memory_allocate(graphics_context->memory_capacity);

    // Ensure these are always a POW2
    graphics_context->shader_capacity            = 4096; // TODO(jerry)
    graphics_context->texture_capacity           = limits.textures;
    graphics_context->font_capacity              = limits.fonts;
    graphics_context->spare_glyph_cache_capacity = limits.glyph_cache;
    graphics_context->quad_batch_capacity        = limits.batch_quads;

    graphics_context->immediate_vertices_count = 0;
    graphics_context->immediate_indices_count  = 0;
    graphics_context->quad_batch_count         = 0;
    graphics_context->font_count               = 0;
    graphics_context->texture_count            = 0;

    graphics_context->shaders           = _graphics_context_push_buffer_unaligned_alloc(graphics_context, graphics_context->shader_capacity * sizeof(*graphics_context->shaders));
    graphics_context->textures          = _graphics_context_push_buffer_unaligned_alloc(graphics_context, graphics_context->texture_capacity * sizeof(*graphics_context->textures));
    graphics_context->fonts             = _graphics_context_push_buffer_unaligned_alloc(graphics_context, graphics_context->font_capacity * sizeof(*graphics_context->fonts));
    graphics_context->glyph_owners      = _graphics_context_push_buffer_unaligned_alloc(graphics_context, graphics_context->spare_glyph_cache_capacity * sizeof(*graphics_context->glyph_owners));
    graphics_context->spare_glyph_cache = _graphics_context_push_buffer_unaligned_alloc(graphics_context, graphics_context->spare_glyph_cache_capacity * sizeof(*graphics_context->spare_glyph_cache));

    // NOTE(jerry): This uses... Way more memory.
    graphics_context->batch_vertices = _graphics_context_push_buffer_unaligned_alloc(graphics_context, (graphics_context->quad_batch_capacity*4 + SPACE_FOR_GENERIC_SHAPE_VERTICES) * sizeof(*graphics_context->batch_vertices));
    graphics_context->batch_indices  = _graphics_context_push_buffer_unaligned_alloc(graphics_context, (graphics_context->quad_batch_capacity*6 + SPACE_FOR_GENERIC_SHAPE_VERTICES) * sizeof(*graphics_context->batch_indices));

    // modern opengl means... Shaders
    graphics_context->batch_quads_shader_program = graphics_context_load_shader_from_source(graphics_context, DEFAULT_VERTEX_SOURCE, DEFAULT_FRAGMENT_SOURCE);
    graphics_context->text_shader_program        = graphics_context_load_shader_from_source(graphics_context, DEFAULT_VERTEX_SOURCE, TEXT_FRAGMENT_SOURCE);

    // This assumes you always interleave your data, and I will work with that
    // assumption always, since it's simplest and doesn't really seem to break performance.
    {
        struct graphics_context_vertex_buffer buffer = _opengl_create_vertex_buffer(
            (struct graphics_context_vertex_buffer_description) {
                .vertex_size  = sizeof(struct graphics_context_default_vertex_format),
                .buffer_count = 4 * graphics_context->quad_batch_capacity + SPACE_FOR_GENERIC_SHAPE_VERTICES,
                .buffer_data  = NULL,
                .usage        = GRAPHICS_CONTEXT_VERTEX_BUFFER_USAGE_TYPE_DYNAMIC,

                .index_buffer = {
                    .index_type   = GRAPHICS_CONTEXT_INDEX_BUFFER_INDEX_TYPE_UINT16,
                    .buffer_count = 6 * graphics_context->quad_batch_capacity + SPACE_FOR_GENERIC_SHAPE_VERTICES, 
                    .buffer_data  = NULL,
                    .usage        = GRAPHICS_CONTEXT_VERTEX_BUFFER_USAGE_TYPE_DYNAMIC,
                },

                .format = {
                    .attributes[0] = { .name = "vertex_position", .element_count = 2, .type = GRAPHICS_CONTEXT_VERTEX_ATTRIBUTE_TYPE_FLOAT, },
                    .attributes[1] = { .name = "vertex_texcoord", .element_count = 2, .normalized = true, .type = GRAPHICS_CONTEXT_VERTEX_ATTRIBUTE_TYPE_INT16, .offset = offsetof(struct graphics_context_default_vertex_format, texture_coordinate_u)},
                    .attributes[2] = { .name = "vertex_colors"  , .element_count = 4, .normalized = true, .type = GRAPHICS_CONTEXT_VERTEX_ATTRIBUTE_TYPE_UINT8, .offset = offsetof(struct graphics_context_default_vertex_format, r)},

                    // These extras are meant to be used as flags.
                    /* .attributes[3] = { .name = "extra_set1", .element_count = 4, .type = GRAPHICS_CONTEXT_VERTEX_ATTRIBUTE_TYPE_UINT8, .offset = offsetof(struct graphics_context_default_vertex_format, extra0)}, */
                }
            }
        );

        graphics_context->batch_vertex_buffer = buffer;
    }
    // text vertex buffer
    {
        struct graphics_context_vertex_buffer buffer = _opengl_create_vertex_buffer(
            (struct graphics_context_vertex_buffer_description) {
                .vertex_size  = sizeof(struct graphics_context_default_vertex_format),
                .buffer_count = 4 ,
                .buffer_data  = NULL,
                .usage        = GRAPHICS_CONTEXT_VERTEX_BUFFER_USAGE_TYPE_DYNAMIC,

                .index_buffer = {
                    .index_type   = GRAPHICS_CONTEXT_INDEX_BUFFER_INDEX_TYPE_UINT16,
                    .buffer_count = 6, 
                    .buffer_data  = NULL,
                    .usage        = GRAPHICS_CONTEXT_VERTEX_BUFFER_USAGE_TYPE_DYNAMIC,
                },

                .format = {
                    .attributes[0] = { .name = "vertex_position", .element_count = 2, .type = GRAPHICS_CONTEXT_VERTEX_ATTRIBUTE_TYPE_FLOAT, },
                    .attributes[1] = { .name = "vertex_texcoord", .element_count = 2, .normalized = true, .type = GRAPHICS_CONTEXT_VERTEX_ATTRIBUTE_TYPE_INT16, .offset = offsetof(struct graphics_context_default_vertex_format, texture_coordinate_u)},
                    .attributes[2] = { .name = "vertex_colors"  , .element_count = 4, .normalized = true, .type = GRAPHICS_CONTEXT_VERTEX_ATTRIBUTE_TYPE_UINT8, .offset = offsetof(struct graphics_context_default_vertex_format, r)},
                }
            }
        );

        graphics_context->text_vertex_buffer = buffer;
    }
    _graphics_context_memory_usage_printout(graphics_context);
}

void graphics_context_update_shader_uniform(struct graphics_context* graphics_context, graphics_context_shader_handle shader, char* uniform_name, struct graphics_context_shader_uniform_parameter uniform_data) {
    if (!uniform_data.count) {
        return;
    }

    struct graphics_context_shader* shader_resource = &graphics_context->shaders[shader.id];
    GLuint shader_program_object = shader_resource->program;
    
    switch (uniform_data.type) {
        case GRAPHICS_CONTEXT_SHADER_UNIFORM_PARAMETER_TYPE_FLOAT: {
            switch (uniform_data.count) {
                case 1: {
                    _opengl_shader_uniform_set_float1(shader_program_object, uniform_name, uniform_data.floating_point.values[0]);
                } break;
                case 2: {
                    _opengl_shader_uniform_set_float2(shader_program_object, uniform_name, uniform_data.floating_point.values[0], uniform_data.floating_point.values[1]);
                } break;
                case 3: {
                    _opengl_shader_uniform_set_float3(shader_program_object, uniform_name, uniform_data.floating_point.values[0], uniform_data.floating_point.values[1], uniform_data.floating_point.values[2]);
                } break;
                case 4: {
                    _opengl_shader_uniform_set_float4(shader_program_object, uniform_name, uniform_data.floating_point.values[0], uniform_data.floating_point.values[1], uniform_data.floating_point.values[2], uniform_data.floating_point.values[3]);
                } break;
            }
        } break;

        case GRAPHICS_CONTEXT_SHADER_UNIFORM_PARAMETER_TYPE_INTEGER: {
            switch (uniform_data.count) {
                case 1: {
                    _opengl_shader_uniform_set_integer1(shader_program_object, uniform_name, uniform_data.integer.values[0]);
                } break;
                case 2: {
                    _opengl_shader_uniform_set_integer2(shader_program_object, uniform_name, uniform_data.integer.values[0], uniform_data.integer.values[1]);
                } break;
                case 3: {
                    _opengl_shader_uniform_set_integer3(shader_program_object, uniform_name, uniform_data.integer.values[0], uniform_data.integer.values[1], uniform_data.integer.values[2]);
                } break;
                case 4: {
                    _opengl_shader_uniform_set_integer4(shader_program_object, uniform_name, uniform_data.integer.values[0], uniform_data.integer.values[1], uniform_data.integer.values[2], uniform_data.integer.values[3]);
                } break;
            }
        } break;

        case GRAPHICS_CONTEXT_SHADER_UNIFORM_PARAMETER_TYPE_MATRIX: {
            _opengl_shader_uniform_set_matrix4x4(shader_program_object, uniform_name, uniform_data.count, uniform_data.matrix.data);
        } break;
    }
}

static void _graphics_context_upload_matrix_uniform_to_all_shader_programs(struct graphics_context* graphics_context, char* name, float* matrix) {
    for (size_t shader_index = 1; shader_index <= graphics_context->shader_count; ++shader_index) {
        _opengl_shader_uniform_set_matrix4x4(graphics_context->shaders[shader_index].program, name, 1, matrix);
    }
}

void graphics_context_set_orthographic_projection(struct graphics_context* graphics_context, float left, float top, float right, float bottom, float near, float far) {
    float scale_x       = (2.0f / (right - left));
    float scale_y       = (2.0f / (top - bottom));
    float scale_z       = (-2.0f / (far - near));
    float translation_x = -((right + left) / (right - left));
    float translation_y = -((top + bottom) / (top - bottom));
    float translation_z = -((far + near) / (far - near));

    float orthographic_matrix[] = {
        scale_x, 0,       0,       0,
        0,       scale_y, 0,       0,
        0,       0,      scale_z,  0,
        translation_x,  translation_y,  translation_z, 1
    };

    memcpy(graphics_context->matrices.projection, orthographic_matrix, sizeof(orthographic_matrix));
    _graphics_context_upload_matrix_uniform_to_all_shader_programs(graphics_context, "projection_matrix", graphics_context->matrices.projection);
}

static uint16_t _graphics_context_search_for_empty_texture_slot(struct graphics_context* graphics_context) {
    uint16_t slot_index = 1;

    assertion(graphics_context->texture_count < graphics_context->texture_capacity && "Somehow we tried to prune for texture slots even though we should have no more textures!");
    for(; slot_index < graphics_context->texture_capacity; ++slot_index) {
        struct graphics_context_texture* texture_resource = &graphics_context->textures[slot_index];

        if (texture_resource->status == GRAPHICS_CONTEXT_RESOURCE_STATUS_READY || texture_resource->is_render_target) {
            continue;
        } else {
            return slot_index;
        }
    }

    return 0;
}
static uint16_t _graphics_context_search_for_unloaded_texture_with_pathname_hashed(struct graphics_context* graphics_context, char* file_location) {
    uint64_t                         hash_key            = fnv1a_hash(file_location, cstring_length(file_location));
    uint16_t                         resource_hash_index = hash_key & (graphics_context->texture_capacity - 1);
    struct graphics_context_texture* texture_resource    = &graphics_context->textures[resource_hash_index];

    if (strncmp(texture_resource->file_path, file_location, BLACKIRON_MAX_PLATFORM_PATHNAME_LENGTH-1) == 0) {
        return resource_hash_index;
    }

    // I literally have no idea how or why this can possibly happen, but I'll
    // thank myself if this ever happens.
    assertion(graphics_context->texture_count < graphics_context->texture_capacity && "Somehow we tried to prune for texture slots even though we should have no more textures!");
    {
        while (texture_resource->status == GRAPHICS_CONTEXT_RESOURCE_STATUS_READY || texture_resource->is_render_target) {
            resource_hash_index++; 

            if (resource_hash_index >= graphics_context->texture_capacity) {
                resource_hash_index = 0;
            }

            texture_resource = &graphics_context->textures[resource_hash_index];
        }
    }

    return resource_hash_index;
}

// this is technically not optimal since I think this one will waste the
// most space, but it should guarantee a fit.
int64_t _graphics_optimal_power_of_two_image_size(int width, int height) {
    if (width > height) {
        return next_power_of_two(width);
    } else {
        return next_power_of_two(height);
    }
}

// temporary memory
// move this to common or something and make it take an allocator instead.
enum resample_bitmap_flags {
    RESAMPLE_BITMAP_FLIP_H = BIT(0),
    RESAMPLE_BITMAP_FLIP_V = BIT(1),
};

// This is mostly inlined and unrolled to one level because it would likely be faster in debug
// this was not profiled, and it's not really super inconvenient so eh.

// This image resampler will take a non power of two image, and pad it from it's extents to get a power of two
// image. This is done to support older hardware and webGL or anything that doesn't allow NPOT natively.

// This is done for fonts(their texture glyphs specifically.) or textures when used as resources,
// should not be used on render targets.
static uint8_t* _resample_bitmap_to_next_power_of_two_ext(struct graphics_context* graphics_context, uint32_t square_size, uint8_t* original_buffer, uint32_t original_width, uint32_t original_height, uint8_t components, uint8_t flags) {
    size_t allocation_size = square_size*square_size * components;
    struct graphics_context_allocation_result allocation = graphics_context_push_buffer_allocate_forward(graphics_context, allocation_size);

    // make sure to zero out memory to make sure nothing weird shows up.
    uint8_t* pixel_buffer = allocation.memory;
    memset(pixel_buffer, 0, allocation_size);

    bool flip_h = (flags & RESAMPLE_BITMAP_FLIP_H);
    bool flip_v = (flags & RESAMPLE_BITMAP_FLIP_V);

    int sample_y;
    int sample_x;
    switch (components) {
        case 1: {
            for (int y = 0; y < original_height; ++y) {
                sample_y = y;
                if (flip_v) {
                    sample_y = (original_height - y);
                }

                for (int x = 0; x < original_width; ++x) {
                    sample_x = x;
                    if (flip_h) {
                        sample_x = (original_width - x);
                    }
                    pixel_buffer[y * square_size + x] = original_buffer[sample_y * original_width + sample_x];
                }
            }
        } break;
        case 3: {
            for (int y = 0; y < original_height; ++y) {
                sample_y = y;
                if (flip_v) {
                    sample_y = (original_height - y);
                }

                for (int x = 0; x < original_width; ++x) {
                    sample_x = x;
                    if (flip_h) {
                        sample_x = (original_width - x);
                    }
                    pixel_buffer[y * square_size*3 + x*3 + 0] = original_buffer[sample_y * original_width*3 + sample_x*3 + 0];
                    pixel_buffer[y * square_size*3 + x*3 + 1] = original_buffer[sample_y * original_width*3 + sample_x*3 + 1];
                    pixel_buffer[y * square_size*3 + x*3 + 2] = original_buffer[sample_y * original_width*3 + sample_x*3 + 2];
                }
            }
        } break;
        case 4: {
            for (int y = 0; y < original_height; ++y) {
                sample_y = y;
                if (flip_v) {
                    sample_y = (original_height - y);
                }

                for (int x = 0; x < original_width; ++x) {
                    sample_x = x;
                    if (flip_h) {
                        sample_x = (original_width - x);
                    }

                    pixel_buffer[y * square_size*4 + x*4 + 0] = original_buffer[sample_y * original_width*4 + sample_x*4 + 0];
                    pixel_buffer[y * square_size*4 + x*4 + 1] = original_buffer[sample_y * original_width*4 + sample_x*4 + 1];
                    pixel_buffer[y * square_size*4 + x*4 + 2] = original_buffer[sample_y * original_width*4 + sample_x*4 + 2];
                    pixel_buffer[y * square_size*4 + x*4 + 3] = original_buffer[sample_y * original_width*4 + sample_x*4 + 3];
                }
            }
        } break;
    }

    // As long as we don't write into the pointer this is safe to just "free".
    // TODO(jerry): to be more robust, I may actually want to return this marker and just free it later.
    graphics_context_push_buffer_restore(graphics_context, allocation.restoration_marker);
    return pixel_buffer;
}
static uint8_t* _resample_bitmap_to_next_power_of_two(struct graphics_context* graphics_context, uint32_t square_size, uint8_t* original_buffer, uint32_t original_width, uint32_t original_height, uint8_t components) {
    return _resample_bitmap_to_next_power_of_two_ext(graphics_context, square_size, original_buffer, original_width, original_height, components, 0);
}

uint16_t _graphics_context_search_for_free_shader_slot_index(struct graphics_context* graphics_context) {
    console_printf("want to prune for slot! (shader count: %d)\n", graphics_context->shader_count);
    uint16_t index = 0;

    for (; index < graphics_context->shader_count; ++index) {
        if (graphics_context->shaders[index+1].program == 0) {
            console_printf("pruned slot: %d\n", index);
            break;
        }
    }

    return index+1;
}
static struct graphics_context_shader_result _graphics_context_handle_shader_construction_from_files(char* vertex_shader_location, char* fragment_shader_location) {
    char* vertex_source;
    char* fragment_source;

    if (vertex_shader_location) {
        vertex_source = read_entire_file(vertex_shader_location);
    } else {
        vertex_source = DEFAULT_VERTEX_SOURCE;
    }

    if (fragment_shader_location) {
        fragment_source = read_entire_file(fragment_shader_location);
    } else {
        fragment_source = DEFAULT_FRAGMENT_SOURCE;
    }

    struct graphics_context_shader_result shader_creation = _opengl_create_shader_program(
        (struct graphics_context_shader_description) {
            .vertex_source = vertex_source,
            .fragment_source = fragment_source,
        }
    );

    if (vertex_shader_location) {
        free_entire_file(vertex_source);
    }
    if (fragment_shader_location) {
        free_entire_file(fragment_source);
    }

    return shader_creation;
}
graphics_context_shader_handle  graphics_context_load_shader_from_file(struct graphics_context* graphics_context, char* vertex_shader_location, char* fragment_shader_location) {
    uint16_t slot_index = _graphics_context_search_for_free_shader_slot_index(graphics_context);
    graphics_context->shader_count++;

    struct graphics_context_shader* shader_resource = &graphics_context->shaders[slot_index];
    shader_resource->from_file                      = true;
    shader_resource->status                         = GRAPHICS_CONTEXT_RESOURCE_STATUS_READY;

    if (vertex_shader_location) {
        strncpy(shader_resource->vertex_file_path, vertex_shader_location, BLACKIRON_MAX_PLATFORM_PATHNAME_LENGTH-1);
    }

    if (fragment_shader_location) {
        strncpy(shader_resource->fragment_file_path, fragment_shader_location, BLACKIRON_MAX_PLATFORM_PATHNAME_LENGTH-1);
    }

    struct graphics_context_shader_result shader_creation = _graphics_context_handle_shader_construction_from_files(vertex_shader_location, fragment_shader_location);
    if (shader_creation.error) {
        console_printf("Vertex Error Log:\n%.*s\n",   GRAPHICS_CONTEXT_MAX_SHADER_LOG_LENGTH, shader_creation.vertex_error_log);
        console_printf("Fragment Error Log:\n%.*s\n", GRAPHICS_CONTEXT_MAX_SHADER_LOG_LENGTH, shader_creation.fragment_error_log);
        console_printf("Linkage Error Log:\n%.*s\n",  GRAPHICS_CONTEXT_MAX_SHADER_LOG_LENGTH, shader_creation.linkage_error_log);
        shader_resource->program = 0;
    } else {
        shader_resource->program = shader_creation.program;
        console_printf("Shader was successfully loaded from a file! (prg id: %d)\n", shader_creation.program);
    }

    graphics_context_shader_handle handle_result = {.id = slot_index};

    {
        struct graphics_context_file_hentai_user_data_packet user_data_packet = (struct graphics_context_file_hentai_user_data_packet) {
            .resource_type = GRAPHICS_CONTEXT_RESOURCE_TYPE_SHADER,
            .context       = graphics_context,
            .shader        = handle_result,
        };
        if (vertex_shader_location) {
            file_hentai_stalk_file(shader_resource->vertex_file_path, &user_data_packet, sizeof(user_data_packet), _graphics_context_reload_resource);
        }

        if (fragment_shader_location) {
            file_hentai_stalk_file(shader_resource->fragment_file_path, &user_data_packet, sizeof(user_data_packet), _graphics_context_reload_resource);
        }
    }

    return handle_result;
}
graphics_context_shader_handle  graphics_context_load_shader_from_source(struct graphics_context* graphics_context, char* vertex_shader_source, char* fragment_shader_source) {
    uint16_t slot_index = _graphics_context_search_for_free_shader_slot_index(graphics_context);
    graphics_context->shader_count++;

    struct graphics_context_shader* shader_resource = &graphics_context->shaders[slot_index];

    if (vertex_shader_source == NULL) {
        vertex_shader_source = DEFAULT_VERTEX_SOURCE;
    }

    if (fragment_shader_source == NULL) {
        fragment_shader_source = DEFAULT_FRAGMENT_SOURCE;
    }

    struct graphics_context_shader_result shader_creation = _opengl_create_shader_program(
        (struct graphics_context_shader_description) {
            .vertex_source = vertex_shader_source,
            .fragment_source = fragment_shader_source,
        }
    );

    if (shader_creation.error) {
        console_printf("Vertex Error Log:\n%.*s\n",   GRAPHICS_CONTEXT_MAX_SHADER_LOG_LENGTH, shader_creation.vertex_error_log);
        console_printf("Fragment Error Log:\n%.*s\n", GRAPHICS_CONTEXT_MAX_SHADER_LOG_LENGTH, shader_creation.fragment_error_log);
        console_printf("Linkage Error Log:\n%.*s\n",  GRAPHICS_CONTEXT_MAX_SHADER_LOG_LENGTH, shader_creation.linkage_error_log);
        shader_resource->program = 0;
    } else {
        shader_resource->program = shader_creation.program;
        console_printf("Shader was successfully compiled from source! (prg id: %d)\n", shader_creation.program);
    }

    graphics_context_shader_handle handle_result = {.id = slot_index};
    return handle_result;
}

struct __texture_loading_result {
    int32_t padded_size;
    int32_t width;
    int32_t height;
    GLuint texture_handle;
};
struct __texture_loading_result _graphics_context_handle_loading_texture_from_file_and_padding_to_power_of_two(struct graphics_context* graphics_context, char* file_location) {
    int image_width;
    int image_height;
    int pixel_components;
    uint8_t* image_pixels = stbi_load(file_location, &image_width, &image_height, &pixel_components, 4);

    int64_t image_square_size = _graphics_optimal_power_of_two_image_size(image_width, image_height);
    uint8_t* resampled_pixels = _resample_bitmap_to_next_power_of_two(graphics_context, image_square_size, image_pixels, image_width, image_height, 4);

    GLuint opengl_texture = _opengl_create_new_texture2d(
        (struct graphics_context_texture_description) {.mipmap_level = 0,},
        image_square_size,
        image_square_size,
        resampled_pixels);

    stbi_image_free(image_pixels);

    return (struct __texture_loading_result) {
        .padded_size    = image_square_size,
        .width          = image_width,
        .height         = image_height,
        .texture_handle = opengl_texture,
    };
}

graphics_context_texture_handle graphics_context_allocate_texture_from_memory(struct graphics_context* graphics_context, struct graphics_context_texture_description texture_description, uint16_t width, uint16_t height, uint8_t* pixels) {
    graphics_context_texture_handle  handle_result      = {};
    size_t                           free_texture_index = 0;
    struct graphics_context_texture* texture_resource   = NULL;

    bool no_freeslot = true;
    for (free_texture_index = 1; free_texture_index < graphics_context->texture_capacity; ++free_texture_index) {
        texture_resource = &graphics_context->textures[free_texture_index];

        if (texture_resource->status == GRAPHICS_CONTEXT_RESOURCE_STATUS_UNLOADED) {
            no_freeslot = false;
            break;
        }
    }

    if (no_freeslot) {
        return null_texture;
    }


    // optimally I would resize this to a power of two like I do with every other
    // texture.
    GLuint opengl_texture                = _opengl_create_new_texture2d(texture_description, width, height, pixels);

    texture_resource->status             = GRAPHICS_CONTEXT_RESOURCE_STATUS_READY;
    texture_resource->width              = width;
    texture_resource->height             = height;
    texture_resource->texture_handle     = opengl_texture;
    texture_resource->_power_of_two_size = 0;

    handle_result.id = free_texture_index;
    return handle_result;
}

graphics_context_texture_handle graphics_context_load_texture_from_file(struct graphics_context* graphics_context, char* file_location) {
    graphics_context_texture_handle handle_result = {};

    // TODO(jerry): While I now handle collisions,
    // this does not quite safely handle not finishing as we don't reserve any slots unfortunately.
    // in the future reserve slot 0 for null.
    // NOTE(jerry): While 0 is considered NULL, 0 slot may still be hashed... Whoops!
    if (graphics_context->texture_count >= graphics_context->texture_capacity) {
        console_printf("warning: we have maxed out the texture resources! (%d is the capacity)\n", graphics_context->texture_count);
        return handle_result;
    }

    uint16_t resource_hash_index = _graphics_context_search_for_unloaded_texture_with_pathname_hashed(graphics_context, file_location);
    handle_result.id = resource_hash_index;
    graphics_context->texture_count++;

    struct graphics_context_texture* texture = &graphics_context->textures[resource_hash_index];
    strncpy(texture->file_path, file_location, BLACKIRON_MAX_PLATFORM_PATHNAME_LENGTH-1);
    {
        struct graphics_context_file_hentai_user_data_packet user_data_packet = (struct graphics_context_file_hentai_user_data_packet) {
            .resource_type = GRAPHICS_CONTEXT_RESOURCE_TYPE_TEXTURE,
            .context       = graphics_context,
            .texture       = handle_result,
        };
        file_hentai_stalk_file(texture->file_path, &user_data_packet, sizeof(user_data_packet), _graphics_context_reload_resource);
    }
    texture->status = GRAPHICS_CONTEXT_RESOURCE_STATUS_READY;

    struct __texture_loading_result result = _graphics_context_handle_loading_texture_from_file_and_padding_to_power_of_two(graphics_context, file_location);

    texture->texture_handle     = result.texture_handle;
    texture->width              = result.width;
    texture->height             = result.height;
    texture->_power_of_two_size = result.padded_size;

    return handle_result;
}

char* gl_framebuffer_status_to_string(int id) {
    if (id == GL_FRAMEBUFFER_COMPLETE) {
        return "good framebuffer";
    }

    return "bad framebuffer";
}

// create a default framebuffer at 720p first...
graphics_context_texture_handle graphics_context_create_render_target_for_screen(struct graphics_context* graphics_context) {
    graphics_context_texture_handle handle_result = {};

    handle_result.id = _graphics_context_search_for_empty_texture_slot(graphics_context);

    struct graphics_context_texture* texture_resource = &graphics_context->textures[handle_result.id];
    texture_resource->is_render_target = true;

    int32_t initial_resolution_width  = 1920;
    int32_t initial_resolution_height = 1080;

    GLuint framebuffer_object;
    GLuint framebuffer_color_attachment;
    // TODO(jerry):
    // make the abstraction layer for this thing!

    // TODO(jerry):
    // I don't use the depth buffer yet,
    // but when I do it, I need to add a renderbuffer! (I don't need to access the
    // renderbuffer myself. I at most need it for Z-testing...)

    {
        glGenFramebuffers(1, &framebuffer_object);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_object);

        glGenTextures(1, &framebuffer_color_attachment);
        glBindTexture(GL_TEXTURE_2D, framebuffer_color_attachment);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, initial_resolution_width, initial_resolution_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebuffer_color_attachment, 0);
        printf("%s\n", gl_framebuffer_status_to_string(glCheckFramebufferStatus(GL_FRAMEBUFFER)));
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    texture_resource->framebuffer_handle = framebuffer_object;
    texture_resource->texture_handle     = framebuffer_color_attachment;

    texture_resource->width              = initial_resolution_width;
    texture_resource->height             = initial_resolution_height;
    return handle_result;
}

// this is for a singular glyph.
static void _graphics_context_font_load_fill_glyph(struct graphics_context* graphics_context, struct graphics_context_font_glyph* glyph, uint32_t codepoint, stbtt_fontinfo* font_info, float scale_factor) {
    static uint8_t _temporary_font_glyph_bitmap[2048*2048] = {};
    memset(_temporary_font_glyph_bitmap, 0, 2048*2048);

    // TODO(jerry): kerning for slightly better looking text.
    glyph->codepoint = codepoint;
    {
        int32_t advance_width;
        int32_t left_side_bearing;
        stbtt_GetCodepointHMetrics(font_info, codepoint, &advance_width, &left_side_bearing);

        glyph->advance_width     = advance_width * scale_factor;
        glyph->left_side_bearing = left_side_bearing * scale_factor;
    }

    {
        int32_t bitmap_left;
        int32_t bitmap_top;
        int32_t bitmap_right;
        int32_t bitmap_bottom;
        stbtt_GetCodepointBitmapBox(font_info, codepoint, scale_factor, scale_factor, &bitmap_left, &bitmap_top, &bitmap_right, &bitmap_bottom);

        glyph->bitmap_left   = bitmap_left;
        glyph->bitmap_top    = bitmap_top;
        glyph->bitmap_right  = bitmap_right;
        glyph->bitmap_bottom = bitmap_bottom;

        {
            int32_t bitmap_width  = (bitmap_right - bitmap_left);
            int32_t bitmap_height = (bitmap_bottom - bitmap_top);

            stbtt_MakeCodepointBitmap(font_info, _temporary_font_glyph_bitmap, bitmap_width, bitmap_height, bitmap_width, scale_factor, scale_factor, codepoint);
            int64_t image_square_size = _graphics_optimal_power_of_two_image_size(bitmap_width, bitmap_height);
            uint8_t* resampled_pixels = _resample_bitmap_to_next_power_of_two_ext(graphics_context, image_square_size, _temporary_font_glyph_bitmap, bitmap_width, bitmap_height, 1, 0);

            graphics_context_texture_handle glyph_texture_id = graphics_context_allocate_texture_from_memory(
                graphics_context,
                (struct graphics_context_texture_description) {
                    .internal_format = GRAPHICS_CONTEXT_TEXTURE_FORMAT_R8,
                    .format          = GRAPHICS_CONTEXT_TEXTURE_FORMAT_R8
                },
                image_square_size, image_square_size,
                resampled_pixels
            );

            glyph->texture_handle = glyph_texture_id;
            glyph->_power_of_two_size = image_square_size;
        }
    }
}

struct glyph_font_range {
    uint32_t starting_glyph;
    uint32_t ending_glyph;
};
struct glyph_bitmap {
    struct graphics_context_allocation_result pixel_allocation;
    uint32_t width;
    uint32_t height;
};
// NOTE(jerry): Our only supported range is ASCII characters.
// As those are the only glyphs we guarantee preloading on at the moment.
// I should change this to be like
#if 0
struct graphics_context_font_glyph_range {
    uint32_t first_codepoint;
    uint32_t last_codepoint;
    struct graphics_context_font_glyph* glyphs;
};
struct graphics_context_font {
    // ... original stuff.
    size_t range_count;
    struct graphics_context_font_glyph_range* ranges;
};
#endif
static void _graphics_context_load_font_glyphs_within_range_and_create_atlas(struct graphics_context* graphics_context, stbtt_fontinfo* font_information, struct graphics_context_font* font_resource, float scale_factor, struct glyph_font_range glyph_range) {
    // xor swap
    if (glyph_range.ending_glyph < glyph_range.starting_glyph) {
        glyph_range.ending_glyph ^= glyph_range.starting_glyph;
        glyph_range.starting_glyph ^= glyph_range.ending_glyph;
        glyph_range.ending_glyph ^= glyph_range.starting_glyph;
    }

    size_t                                    amount_of_glyphs = glyph_range.ending_glyph - glyph_range.starting_glyph;
    struct graphics_context_allocation_result allocation    = graphics_context_push_buffer_allocate_forward(graphics_context, amount_of_glyphs * sizeof(struct glyph_bitmap));
    struct glyph_bitmap*                      glyph_bitmaps = allocation.memory;

    enum {
        __FILLING_IN_BITMAPS,
        __FIND_FONT_ATLAS_SIZE,
        __PACK_FONT_ATLAS,
        __END_OF_LOADING,
    };

    uint8_t loading_stage = __FILLING_IN_BITMAPS;

    uint32_t current_power_of_two = 16;
    uint32_t pack_cursor_x        = 0;
    uint32_t pack_cursor_y        = 0;

    struct graphics_context_allocation_result bitmap_allocation;
start_of_glyph_iteration:
    pack_cursor_x = 0;
    pack_cursor_y = 0;

    if (loading_stage == __PACK_FONT_ATLAS) {
        console_printf("best size: %d\n", current_power_of_two);
        bitmap_allocation = graphics_context_push_buffer_allocate_forward(graphics_context, current_power_of_two * current_power_of_two);
    }

    for (size_t bitmap_index = 0; bitmap_index < amount_of_glyphs; ++bitmap_index) {
        uint32_t                            current_codepoint = glyph_range.starting_glyph + bitmap_index;
        struct graphics_context_font_glyph* current_glyph     = &font_resource->glyphs[current_codepoint];
        struct glyph_bitmap*                current_bitmap    = &glyph_bitmaps[bitmap_index];

        switch (loading_stage) {
            case __FILLING_IN_BITMAPS: {
                int32_t advance_width;
                int32_t left_side_bearing;

                stbtt_GetCodepointHMetrics(font_information, current_codepoint, &advance_width, &left_side_bearing);

                int32_t bitmap_left;
                int32_t bitmap_top;
                int32_t bitmap_right;
                int32_t bitmap_bottom;

                stbtt_GetCodepointBitmapBox(font_information, current_codepoint, scale_factor, scale_factor, &bitmap_left, &bitmap_top, &bitmap_right, &bitmap_bottom);

                int32_t bitmap_width  = (bitmap_right - bitmap_left);
                int32_t bitmap_height = (bitmap_bottom - bitmap_top);

                {
                    current_bitmap->width            = bitmap_width;
                    current_bitmap->height           = bitmap_height;
                    current_bitmap->pixel_allocation = graphics_context_push_buffer_allocate_forward(graphics_context, bitmap_width * bitmap_height);

                    uint8_t* pixel_memory = current_bitmap->pixel_allocation.memory;
                    stbtt_MakeCodepointBitmap(font_information, pixel_memory, bitmap_width, bitmap_height, bitmap_width, scale_factor, scale_factor, current_codepoint);

                    // preliminary fill in.
                    current_glyph->bitmap_left       = bitmap_left;
                    current_glyph->bitmap_top        = bitmap_top;
                    current_glyph->bitmap_right      = bitmap_right;
                    current_glyph->bitmap_bottom     = bitmap_bottom;
                    current_glyph->advance_width     = advance_width * scale_factor;
                    current_glyph->left_side_bearing = left_side_bearing * scale_factor;
                }
            } break;
            case __FIND_FONT_ATLAS_SIZE: {
                // The algorithm for this isn't anything clever. It's a stupid bruteforcer,
                // all I do is I keep trying powers of two until I find that it fits.
                if (pack_cursor_x + current_glyph->advance_width >= current_power_of_two) {
                    pack_cursor_x  = 0;
                    pack_cursor_y += font_resource->font_size;
                } else {
                    pack_cursor_x += current_glyph->advance_width;
                }

                // Failed to pack for this iteration... Try again with the next power of two.
                if (pack_cursor_y >= current_power_of_two) {
                    current_power_of_two <<= 1;
                    goto start_of_glyph_iteration;
                }
            } break;
            case __PACK_FONT_ATLAS: {
                uint8_t* bitmap_atlas_memory = bitmap_allocation.memory;
                uint8_t* bitmap_pixel_buffer = current_bitmap->pixel_allocation.memory;

                {
                    if ((pack_cursor_x + current_bitmap->width) >= current_power_of_two) {
                        pack_cursor_x  = 0;
                        pack_cursor_y += font_resource->font_size;
                    }
                
                    current_glyph->texture_coordinate_x = pack_cursor_x;
                    current_glyph->texture_coordinate_y = pack_cursor_y;
                    current_glyph->texture_coordinate_w = current_bitmap->width;
                    current_glyph->texture_coordinate_h = current_bitmap->height;

                    for (size_t y = 0; y < current_bitmap->height; ++y) {
                        for (size_t x = 0; x < current_bitmap->width; ++x) {
                            bitmap_atlas_memory[(pack_cursor_y + y) * current_power_of_two + (pack_cursor_x + x)] = bitmap_pixel_buffer[y * current_bitmap->width + x];
                        }
                    }

                    pack_cursor_x += current_bitmap->width;
                }
            } break;
        }
    }

    loading_stage++;

    if (loading_stage != __END_OF_LOADING) {
        goto start_of_glyph_iteration;
    }

    // generate opengl texture.
    {
        graphics_context_texture_handle opengl_texture = graphics_context_allocate_texture_from_memory(
            graphics_context,
            (struct graphics_context_texture_description) {
                .internal_format = GRAPHICS_CONTEXT_TEXTURE_FORMAT_R8,
                .format = GRAPHICS_CONTEXT_TEXTURE_FORMAT_R8,
                .unpack_alignment = 1,
            },
            current_power_of_two,
            current_power_of_two,
            bitmap_allocation.memory
        );

        for (size_t glyph_index = 0; glyph_index < 128; ++glyph_index) {
            struct graphics_context_font_glyph* current_glyph = &font_resource->glyphs[glyph_index];
            current_glyph->texture_handle                     = opengl_texture;
            current_glyph->_power_of_two_size                 = current_power_of_two;
        }
    }

    // stack ordered deallocation.
    graphics_context_push_buffer_restore(graphics_context, bitmap_allocation.restoration_marker);
    for (size_t bitmap_index = amount_of_glyphs-1; bitmap_index != (size_t)(-1); --bitmap_index) {
        struct glyph_bitmap* current_bitmap = &glyph_bitmaps[bitmap_index];
        graphics_context_push_buffer_restore(graphics_context, current_bitmap->pixel_allocation.restoration_marker);
    }
    graphics_context_push_buffer_restore(graphics_context, allocation.restoration_marker);
}

static void* _graphics_context_font_resources_attempt_to_reuse_existing_file_buffer(struct graphics_context* graphics_context, char* file_location) {
    for (unsigned font_index = 0; font_index < graphics_context->font_capacity; ++font_index) {
        struct graphics_context_font* current_font = &graphics_context->fonts[font_index];
        if (current_font->file_buffer) {
            if (strncmp(current_font->file_path, file_location, BLACKIRON_MAX_PLATFORM_PATHNAME_LENGTH-1) == 0) {
                return current_font->file_buffer;
            }
        }
    }

    return NULL;
}

// NOTE(jerry): Font size shouldn't be a float, but it doesn't really do anything so whatever.
static uint16_t _graphics_context_search_for_unloaded_texture_with_pathname_and_font_size_hashed(struct graphics_context* graphics_context, float font_size, char* file_location) {
    uint64_t hash_key = fnv1a_hash(file_location, cstring_length(file_location)) + fnv1a_hash(&font_size, sizeof(float));
    uint16_t resource_hash_index = hash_key & (graphics_context->font_capacity - 1);
    struct graphics_context_font* font_resource = &graphics_context->fonts[resource_hash_index]; 
    
    if (strncmp(font_resource->file_path, file_location, BLACKIRON_MAX_PLATFORM_PATHNAME_LENGTH) == 0 && font_resource->font_size == font_size) {
        return resource_hash_index;
    }

    assertion(graphics_context->font_count < graphics_context->font_capacity && "Somehow we tried to prune for unused font slots, when we don't have anymore fonts!");
    {

        // this should really be != UNLOADED, but I don't have async io so that's not an issue.
        while (font_resource->status == GRAPHICS_CONTEXT_RESOURCE_STATUS_READY) {
            resource_hash_index++;

            if (resource_hash_index >= graphics_context->font_capacity) {
                resource_hash_index = 0;
            }

            font_resource = &graphics_context->fonts[resource_hash_index];
        }
    }

    return resource_hash_index;
}

graphics_context_font_handle graphics_context_load_font_from_file(struct graphics_context* graphics_context, float font_size, char* file_location) {
    graphics_context_font_handle handle_result;
    uint64_t hash_key = fnv1a_hash(file_location, cstring_length(file_location)) + fnv1a_hash(&font_size, sizeof(float));
    uint16_t resource_hash_index = _graphics_context_search_for_unloaded_texture_with_pathname_and_font_size_hashed(graphics_context, font_size, file_location);

    graphics_context->font_count++;
    handle_result.id = resource_hash_index;

    console_printf("loading: %s as a font\n", file_location);
    stbtt_fontinfo font;

    // as font resources may only differ by size, we can just search for an existing font buffer
    // that we can simply reuse. To avoid wasting memory.
    uint8_t* file_buffer = _graphics_context_font_resources_attempt_to_reuse_existing_file_buffer(graphics_context, file_location);
    if (file_buffer == NULL) {
        file_buffer = _graphics_context_read_file_into_buffer(graphics_context, file_location);
    }


    if (!stbtt_InitFont(&font, file_buffer, 0)) {
        return (graphics_context_font_handle){ .id = 0 };
    }

    struct graphics_context_font* font_resource = &graphics_context->fonts[resource_hash_index];

    font_resource->file_buffer = file_buffer;
    strncpy(font_resource->file_path, file_location, BLACKIRON_MAX_PLATFORM_PATHNAME_LENGTH-1);
    font_resource->font_size = font_size;
    font_resource->status    = GRAPHICS_CONTEXT_RESOURCE_STATUS_READY;

    // This is simple for now, since this usually looked okay...
    uint32_t prebaked_font_size = (uint32_t)font_size;
    float scale_factor = stbtt_ScaleForPixelHeight(&font, prebaked_font_size);
    {
        int32_t glyph_ascent;

        int32_t glyph_descent;
        int32_t line_gap;
        stbtt_GetFontVMetrics(&font, &glyph_ascent, &glyph_descent, &line_gap);

        font_resource->ascent   = glyph_ascent * scale_factor;
        font_resource->descent  = glyph_descent * scale_factor;
        font_resource->line_gap = line_gap * scale_factor;
    }

    _graphics_context_load_font_glyphs_within_range_and_create_atlas(graphics_context, &font, font_resource, scale_factor, (struct glyph_font_range) {.starting_glyph = 0, .ending_glyph = 128});

    return handle_result;
}

graphics_context_font_handle graphics_context_load_font_from_buffer_and_keyed_as(struct graphics_context* graphics_context, void* baked_font_file, float font_size, char* key_name) {
    graphics_context_font_handle handle_result;
    uint64_t hash_key = fnv1a_hash(key_name, cstring_length(key_name)) + fnv1a_hash(&font_size, sizeof(float));
    uint16_t resource_hash_index = _graphics_context_search_for_unloaded_texture_with_pathname_and_font_size_hashed(graphics_context, font_size, key_name);

    graphics_context->font_count++;
    handle_result.id = resource_hash_index;

    stbtt_fontinfo font;

    // as font resources may only differ by size, we can just search for an existing font buffer
    // that we can simply reuse. To avoid wasting memory.
    uint8_t* file_buffer = baked_font_file;

    if (!stbtt_InitFont(&font, file_buffer, 0)) {
        return (graphics_context_font_handle){ .id = 0 };
    }

    struct graphics_context_font* font_resource = &graphics_context->fonts[resource_hash_index];

    font_resource->file_buffer = file_buffer;
    /* strncpy(font_resource->file_path, file_location, BLACKIRON_MAX_PLATFORM_PATHNAME_LENGTH-1); */
    font_resource->font_size = font_size;
    font_resource->status    = GRAPHICS_CONTEXT_RESOURCE_STATUS_READY;

    // This is simple for now, since this usually looked okay...
    uint32_t prebaked_font_size = (uint32_t)font_size;
    float scale_factor = stbtt_ScaleForPixelHeight(&font, prebaked_font_size);
    {
        int32_t glyph_ascent;
        int32_t glyph_descent;
        int32_t line_gap;
        stbtt_GetFontVMetrics(&font, &glyph_ascent, &glyph_descent, &line_gap);

        font_resource->ascent   = glyph_ascent * scale_factor;
        font_resource->descent  = glyph_descent * scale_factor;
        font_resource->line_gap = line_gap * scale_factor;
    }

    _graphics_context_load_font_glyphs_within_range_and_create_atlas(graphics_context, &font, font_resource, scale_factor, (struct glyph_font_range) {.starting_glyph = 0, .ending_glyph = 128});

    return handle_result;
}

// commands / graphics_context actions
void graphics_context_set_viewport(struct graphics_context* graphics_context, float x, float y, float w, float h) {
    glViewport(x, y, w, h);
}

void _opengl_vertex_buffer_update_portion(struct graphics_context_vertex_buffer* vertex_buffer, size_t offset_start, size_t amount, void* data) {
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer->vertex_buffer_object);
    glBufferSubData(GL_ARRAY_BUFFER, offset_start, amount * vertex_buffer->description.vertex_size, data);
}
void _opengl_vertex_buffer_update_portion_index(struct graphics_context_vertex_buffer* vertex_buffer, size_t offset_start, size_t amount, void* data) {
    if (vertex_buffer->description.index_buffer.buffer_count > 0) {
        size_t element_size = _map_index_buffer_index_type_enum_into_element_size(vertex_buffer->description.index_buffer.index_type);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertex_buffer->index_buffer_object);
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset_start, amount * element_size, data);
    }
}
void _opengl_draw_buffer(struct graphics_context_vertex_buffer* vertex_buffer, GLuint texture_id, GLuint shader_program, size_t offset_start, size_t amount) {
    glUseProgram(shader_program);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    glBindVertexArray(vertex_buffer->vertex_array_object);
    glDrawArrays(GL_TRIANGLES, offset_start, amount);
}

void _opengl_draw_buffer_index(struct graphics_context_vertex_buffer* vertex_buffer, GLuint texture_id, GLuint shader_program, size_t offset_start, size_t amount) {
    glUseProgram(shader_program);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    glBindVertexArray(vertex_buffer->vertex_array_object);
    /*
      NOTE(jerry):
      grrrr, what the fuck???
      "The index buffer binding is stored within the VAO. If no VAO is bound, then you cannot bind a buffer object to GL_ELEMENT_ARRAY_BUFFER. "
      
      INDEX BUFFER BINDING STORED WITHIN VAO...

      Why do I have to rebind it then? Is this just an Nvidia thing?
    */
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertex_buffer->index_buffer_object);
    glDrawElements(GL_TRIANGLES, amount, _map_index_buffer_index_type_enum_into_opengl(vertex_buffer->description.index_buffer.index_type), (void*)offset_start);
}

static void _graphics_context_batch_flush_all_quads(struct graphics_context* graphics_context) {
    if (graphics_context->quad_batch_count == 0) {
        return;
    }

    uint16_t texture_id = graphics_context->current_batch.texture.id;
    struct graphics_context_texture* texture_resource = &graphics_context->textures[texture_id];

    {
        GLuint shader_program = 0;

        if (graphics_context->current_batch.shader.id == 0) {
            shader_program = graphics_context->shaders[graphics_context->batch_quads_shader_program.id].program;
        } else {
            shader_program = graphics_context->shaders[graphics_context->current_batch.shader.id].program;
        }

        {
            _opengl_shader_uniform_set_integer1(shader_program, "sampler_texture", 0);
            _opengl_shader_uniform_set_float1(shader_program, "using_texture", (float)(texture_id != 0));
        }

        size_t full_buffer_count = 4 * graphics_context->quad_batch_count;
        size_t full_index_count  = 6 * graphics_context->quad_batch_count;
        _opengl_vertex_buffer_update_portion(&graphics_context->batch_vertex_buffer, 0, full_buffer_count, graphics_context->batch_vertices);
        _opengl_vertex_buffer_update_portion_index(&graphics_context->batch_vertex_buffer, 0, full_index_count, graphics_context->batch_indices);
        _opengl_draw_buffer_index(&graphics_context->batch_vertex_buffer, texture_resource->texture_handle, shader_program, 0, full_index_count);
    }

    graphics_context->quad_batch_count = 0;
}

void _graphics_context_batch_flush_all_immediate_vertices(struct graphics_context* graphics_context) {
    if (graphics_context->immediate_vertices_count == 0) {
        return;
    }

    GLuint shader_program = graphics_context->shaders[graphics_context->batch_quads_shader_program.id].program;

    _opengl_shader_uniform_set_float1(shader_program, "using_texture", 0.0);

    size_t full_buffer_count = graphics_context->immediate_vertices_count;
    size_t full_index_count  = graphics_context->immediate_indices_count;

    _opengl_vertex_buffer_update_portion(&graphics_context->batch_vertex_buffer, 0, full_buffer_count,      graphics_context->batch_vertices + (graphics_context->quad_batch_capacity*4));
    _opengl_vertex_buffer_update_portion_index(&graphics_context->batch_vertex_buffer, 0, full_index_count, graphics_context->batch_indices  + (graphics_context->quad_batch_capacity*6));
    _opengl_draw_buffer_index(&graphics_context->batch_vertex_buffer, 0, shader_program, 0, full_index_count);

    graphics_context->immediate_vertices_count = 0;
    graphics_context->immediate_indices_count  = 0;
}

void _graphics_context_flush_all(struct graphics_context* graphics_context) {
    _graphics_context_batch_flush_all_quads(graphics_context);
    _graphics_context_batch_flush_all_immediate_vertices(graphics_context);
}

// These are only macros to guarantee inlining.
#define __graphics_float2_rotate_by(v, theta)                           \
    do {                                                                \
        float original_vx = (v)[0];                                     \
        (v)[0] = ((v)[0] * cosf(theta))      - ((v)[1] * sinf(theta));  \
        (v)[1] = (original_vx * sinf(theta)) + ((v)[1] * cosf(theta));  \
    } while(0); 
#define __graphics_float2_subtract(v, x, y)     \
    do {                                        \
        (v)[0] -= (x);                          \
        (v)[1] -= (y);                          \
    } while(0); 
#define __graphics_float2_add(v, x, y)          \
    do {                                        \
        (v)[0] += (x);                          \
        (v)[1] += (y);                          \
    } while(0); 

static void _graphics_context_push_batch_quad(struct graphics_context* graphics_context, graphics_context_texture_handle texture, struct graphics_context_batch_quad quad) {
    float quad_min_x = quad.x;
    float quad_max_x = quad.x + quad.w;
    float quad_min_y = quad.y;
    float quad_max_y = quad.y + quad.h;

    bool x_overlap = quad_min_x < graphics_context->screen_bounds.right && graphics_context->screen_bounds.left < quad_max_x;
    bool y_overlap = quad_min_y < graphics_context->screen_bounds.bottom && graphics_context->screen_bounds.top < quad_max_y;

    bool fits_in_screen = x_overlap && y_overlap;
    if (fits_in_screen) {
        bool should_flush = false;

        if (graphics_context->current_batch.texture.id != texture.id) {
            should_flush = true;
        }

        if (graphics_context->current_batch.shader.id != quad.shader.id) {
            should_flush = true;
        }

        if (graphics_context->quad_batch_count >= graphics_context->quad_batch_capacity) {
            should_flush = true;
        }

        if (should_flush) {
            _graphics_context_batch_flush_all_quads(graphics_context);
            graphics_context->current_batch.texture = texture;
            graphics_context->current_batch.shader  = quad.shader;
        }

        // assume they meant the whole texture
        if (quad.uv_w == 0 &&
            quad.uv_h == 0 &&
            quad.uv_x == 0 &&
            quad.uv_y == 0) {
            quad.uv_w = 1;
            quad.uv_h = 1;
        }

        {
            uint16_t texture_id = graphics_context->current_batch.texture.id;
            struct graphics_context_texture* texture_resource = &graphics_context->textures[texture_id];

            if (texture_id) {
                // OpenGL for some reason specifies texture coordinates
                // as starting from the bottom left...
                // (well they specify lots of images starting from origin bottom left coordinate system)

                // This is really annoying because it defies convention so this has to be put in to get what I expect
                // to see.
                if (!texture_resource->is_render_target) {
                    float pot_size = texture_resource->_power_of_two_size;

                    float uv_scale_x = texture_resource->width / pot_size;
                    float uv_scale_y = texture_resource->height / pot_size;

                    if (pot_size == 0.0) {
                        uv_scale_x = 1;
                        uv_scale_y = 1;
                    }

                    quad.uv_x *= uv_scale_x;
                    quad.uv_y *= uv_scale_y;
                    quad.uv_w *= uv_scale_x;
                    quad.uv_h *= uv_scale_y;
                } else {
                    quad.uv_h *= -1;
                }
            }

            float top_left[4] = {
                quad.x,    quad.y,
                quad.uv_x, quad.uv_y
            };
            float top_right[4] = {
                quad.x    + quad.w,    quad.y,
                quad.uv_x + quad.uv_w, quad.uv_y,
            };
            float bottom_left[4] = {
                quad.x,    quad.y    + quad.h,
                quad.uv_x, quad.uv_y + quad.uv_h,
            };
            float bottom_right[4] = {
                quad.x    + quad.w,    quad.y    + quad.h,
                quad.uv_x + quad.uv_w, quad.uv_y + quad.uv_h,
            };

            float rotation_pivot_x = texture_resource->width  * quad.rotation_pivot_x;
            float rotation_pivot_y = texture_resource->height * quad.rotation_pivot_y;

            // apply rotation here
            __graphics_float2_subtract(top_left,     quad.x + (rotation_pivot_x), quad.y + (rotation_pivot_y));
            __graphics_float2_subtract(top_right,    quad.x + (rotation_pivot_x), quad.y + (rotation_pivot_y));
            __graphics_float2_subtract(bottom_left,  quad.x + (rotation_pivot_x), quad.y + (rotation_pivot_y));
            __graphics_float2_subtract(bottom_right, quad.x + (rotation_pivot_x), quad.y + (rotation_pivot_y));
            {
                float theta = degree_to_radians(quad.rotation_degrees);
                {
                    __graphics_float2_rotate_by(top_left,  theta);
                    __graphics_float2_rotate_by(top_right, theta);
                    __graphics_float2_rotate_by(bottom_left,  theta);
                    __graphics_float2_rotate_by(bottom_right,  theta);
                }
            }
            __graphics_float2_add(top_left,     quad.x + (rotation_pivot_x), quad.y + (rotation_pivot_y));
            __graphics_float2_add(top_right,    quad.x + (rotation_pivot_x), quad.y + (rotation_pivot_y));
            __graphics_float2_add(bottom_left,  quad.x + (rotation_pivot_x), quad.y + (rotation_pivot_y));
            __graphics_float2_add(bottom_right, quad.x + (rotation_pivot_x), quad.y + (rotation_pivot_y));

            // I would like to just negate these, however negating this does not work with
            // text quads
            {
                if (quad.flags & GRAPHICS_CONTEXT_FLIP_HORIZONTAL) {
                    Swap(float,    top_left[2],    top_right[2]);
                    Swap(float, bottom_left[2], bottom_right[2]);
                }

                if (quad.flags & GRAPHICS_CONTEXT_FLIP_VERTICAL) {
                    Swap(float,    top_left[3],    top_right[3]);
                    Swap(float, bottom_left[3], bottom_right[3]);
                }
            }

            uint8_t rgba[4];
            decode_rgba_from_uint32_unnormalized(quad.rgba, rgba);

            // push into the humungous vertex buffer (the real one, not the object)
            top_left[2]     *= 65535 / 2;
            top_left[3]     *= 65535 / 2;
            top_right[2]    *= 65535 / 2;
            top_right[3]    *= 65535 / 2;
            bottom_right[2] *= 65535 / 2;
            bottom_right[3] *= 65535 / 2;
            bottom_left[2]  *= 65535 / 2;
            bottom_left[3]  *= 65535 / 2;

            graphics_context->batch_vertices[graphics_context->quad_batch_count*4 + 0] = (struct graphics_context_default_vertex_format) {     top_left[0],     top_left[1],     top_left[2],     top_left[3], rgba[0], rgba[1], rgba[2], rgba[3] };
            graphics_context->batch_vertices[graphics_context->quad_batch_count*4 + 1] = (struct graphics_context_default_vertex_format) {  bottom_left[0],  bottom_left[1],  bottom_left[2],  bottom_left[3], rgba[0], rgba[1], rgba[2], rgba[3] };
            graphics_context->batch_vertices[graphics_context->quad_batch_count*4 + 2] = (struct graphics_context_default_vertex_format) {    top_right[0],    top_right[1],    top_right[2],    top_right[3], rgba[0], rgba[1], rgba[2], rgba[3] };
            graphics_context->batch_vertices[graphics_context->quad_batch_count*4 + 3] = (struct graphics_context_default_vertex_format) { bottom_right[0], bottom_right[1], bottom_right[2], bottom_right[3], rgba[0], rgba[1], rgba[2], rgba[3] };

            uint16_t top_left_index     = (graphics_context->quad_batch_count*4) + 0;
            uint16_t top_right_index    = (graphics_context->quad_batch_count*4) + 2;
            uint16_t bottom_left_index  = (graphics_context->quad_batch_count*4) + 1;
            uint16_t bottom_right_index = (graphics_context->quad_batch_count*4) + 3;

            graphics_context->batch_indices[graphics_context->quad_batch_count*6 + 0] = top_left_index;
            graphics_context->batch_indices[graphics_context->quad_batch_count*6 + 1] = bottom_left_index;
            graphics_context->batch_indices[graphics_context->quad_batch_count*6 + 2] = top_right_index;
            graphics_context->batch_indices[graphics_context->quad_batch_count*6 + 3] = top_right_index;
            graphics_context->batch_indices[graphics_context->quad_batch_count*6 + 4] = bottom_left_index;
            graphics_context->batch_indices[graphics_context->quad_batch_count*6 + 5] = bottom_right_index;

            graphics_context->quad_batch_count++;
        }
    }
}


void graphics_context_bind_render_target(struct graphics_context* graphics_context, graphics_context_texture_handle render_target_texture) {
    graphics_context->currently_bound_render_target = render_target_texture;
}

void graphics_context_begin_drawing(struct graphics_context* graphics_context, struct camera view) {
    float screen_width  = graphics_context->screen_width;
    float screen_height = graphics_context->screen_height;

    float virtual_width  = graphics_context->virtual_resolution.width;
    float virtual_height = graphics_context->virtual_resolution.height;

    // NOTE(jerry):
    // feel free to clean this up later. But it's not super important.
    // 
    bool bound_render_target = false;
    {
        struct graphics_context_texture* render_target = &graphics_context->textures[graphics_context->currently_bound_render_target.id];
        if (graphics_context->currently_bound_render_target.id != 0) {
            assertion(render_target->is_render_target);
            glBindFramebuffer(GL_FRAMEBUFFER, render_target->framebuffer_handle);

            screen_width  = render_target->width;
            screen_height = render_target->height;

            virtual_width  = screen_width;
            virtual_height = screen_height;
        } else {
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
    }
    graphics_context_set_orthographic_projection(graphics_context, 0, 0, screen_width, screen_height, -0.1, 1000.0);

    // setup any state if at all. Otherwise treat this as a "semantic" call
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_SCISSOR_TEST);
    glScissor(0, 0, screen_width, screen_height);

    // alpha blending
    // (S * A) + D * (1 - A) or lerp(src, dest, alpha);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // default initialize. Virtual resolution is just the real resolution.
    if (virtual_width == 0 && virtual_height == 0) {
        virtual_width  = screen_width;
        virtual_height = screen_height;
    }

    if (view.scale_x == 0) {
        view.scale_x = 1;
    }

    if (view.scale_y == 0) {
        view.scale_y = 1;
    }

    struct graphics_context_viewport_rectangle aspect_ratio_fitting_rectangle = _viewport_rectangle_for_virtual_resolution(screen_width, screen_height, virtual_width, virtual_height);

    float view_x = view.x;
    float view_y = view.y;

    {
        float translation_x = -view_x;
        float translation_y = -view_y;
        
        float scale_x = view.scale_x;
        float scale_y = view.scale_y;

        // screen culling
        graphics_context->screen_bounds.left   = view_x / scale_x - 150;
        graphics_context->screen_bounds.right  = graphics_context->screen_bounds.left + virtual_width / scale_x + 150;
        graphics_context->screen_bounds.top    = view_y / scale_y - 150;
        graphics_context->screen_bounds.bottom = graphics_context->screen_bounds.top + virtual_height / scale_y + 150;

        scale_x *= (screen_width/virtual_width);
        scale_y *= (screen_height/virtual_height);

        translation_x *= (screen_width/virtual_width);
        translation_y *= (screen_height/virtual_height);

        graphics_context_set_viewport(graphics_context,
                                      floor(aspect_ratio_fitting_rectangle.x),
                                      floor(aspect_ratio_fitting_rectangle.y),
                                      floor(aspect_ratio_fitting_rectangle.w),
                                      floor(aspect_ratio_fitting_rectangle.h));

        float view_matrix[] = {
            scale_x, 0, 0, 0,
            0, scale_y, 0, 0,
            0, 0,       1, 0,
            translation_x, translation_y, 0, 1,
        };
        memcpy(graphics_context->matrices.view, view_matrix, sizeof(view_matrix));
        

        // All shaders need to be synchronized on commonly shared uniforms like projection and friends.
        _graphics_context_upload_matrix_uniform_to_all_shader_programs(graphics_context, "view_matrix", graphics_context->matrices.view);
    }
}

void graphics_context_end_drawing(struct graphics_context* graphics_context) {
    _graphics_context_flush_all(graphics_context);

    glDisable(GL_BLEND);
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_TEXTURE_2D);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// this is not deferred and happens immediately.
void graphics_context_clear_buffer(struct graphics_context* graphics_context, float r, float g, float b, float a) {
    // make this deferred.
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT);
}

void graphics_context_set_scissor_region(struct graphics_context* graphics_context, float x, float y, float w, float h) {
    struct graphics_context_viewport_rectangle viewport = _viewport_rectangle_for_virtual_resolution(graphics_context->screen_width, graphics_context->screen_height, graphics_context->virtual_resolution.width, graphics_context->virtual_resolution.height);
    bool whole_screen = (x == 0 && y == 0 && w == 0 && h == 0);

    float scissor_x = 0;
    float scissor_y = 0;
    float scissor_w = graphics_context->virtual_resolution.width;
    float scissor_h = graphics_context->virtual_resolution.height;

    if (scissor_w == 0 && scissor_h == 0) {
        scissor_w = graphics_context->screen_width;
        scissor_h = graphics_context->screen_height;
    }

    if (!whole_screen) {
        scissor_x = x;
        scissor_y = scissor_h - (y+h);
        scissor_w = w;
        scissor_h = h;
    }

    // Fit it into the location of the virtual resolution viewport. This isn't the
    // virtual resolution space though. This is still in screenspace.
    {
        scissor_x += viewport.x;
        scissor_y += viewport.y;

        scissor_x *= viewport.scale_factor;
        scissor_y *= viewport.scale_factor;

        scissor_w *= viewport.scale_factor;
        scissor_h *= viewport.scale_factor;
    }

    glScissor(scissor_x, scissor_y, scissor_w, scissor_h);
}

// This should be deferred, for now we're not deferring
void graphics_context_draw_untextured_quad(struct graphics_context* graphics_context, float x, float y, float w, float h, float r, float g, float b, float a, graphics_context_shader_handle shader) {
    _graphics_context_push_batch_quad(
        graphics_context,
        (graphics_context_texture_handle) {
            .id = 0
        },
        (struct graphics_context_batch_quad) {
            .x = x,
            .y = y,
            .w = w,
            .h = h,
            .rgba   = encode_rgba_from_float(r, g, b, a),
            .shader = shader,
        }
    );
}

void graphics_context_draw_untextured_quad_with_rotation(struct graphics_context* graphics_context, float x, float y, float w, float h, float rotation_pivot_x, float rotation_pivot_y, float degrees, float r, float g, float b, float a, graphics_context_shader_handle shader) {
    _graphics_context_push_batch_quad(
        graphics_context,
        (graphics_context_texture_handle) {
            .id = 0
        },
        (struct graphics_context_batch_quad) {
            .x = x,
            .y = y,
            .w = w,
            .h = h,

            .rgba   = encode_rgba_from_float(r, g, b, a),
            .shader = shader,

            .rotation_pivot_x = rotation_pivot_x,
            .rotation_pivot_y = rotation_pivot_y,
            .rotation_degrees = degrees,
        }
    );
}

void graphics_context_draw_textured_quad(struct graphics_context* graphics_context, graphics_context_texture_handle texture, float x, float y, float w, float h, float r, float g, float b, float a, graphics_context_shader_handle shader) {
    _graphics_context_push_batch_quad(
        graphics_context,
        texture,
        (struct graphics_context_batch_quad) {
            .x = x,
            .y = y,
            .w = w,
            .h = h,
            .rgba   = encode_rgba_from_float(r, g, b, a),
            .shader = shader,
        }
    );
}

void graphics_context_draw_textured_quad_with_rotation(struct graphics_context* graphics_context, graphics_context_texture_handle texture, float x, float y, float w, float h, float rotation_pivot_x, float rotation_pivot_y, float degrees, float r, float g, float b, float a, graphics_context_shader_handle shader) {
    _graphics_context_push_batch_quad(
        graphics_context,
        texture,
        (struct graphics_context_batch_quad) {
            .x = x,
            .y = y,
            .w = w,
            .h = h,

            .rgba   = encode_rgba_from_float(r, g, b, a),
            .shader = shader,

            .rotation_pivot_x = rotation_pivot_x,
            .rotation_pivot_y = rotation_pivot_y,
            .rotation_degrees = degrees,
        }
    );
}

void graphics_context_draw_textured_quad_ext(struct graphics_context* graphics_context, graphics_context_texture_handle texture, float x, float y, float w, float h, float ux, float uy, float uw, float uh, uint8_t flags, float r, float g, float b, float a, graphics_context_shader_handle shader) {
    struct graphics_context_texture* texture_resource = &graphics_context->textures[texture.id];

    uint32_t texture_width  = texture_resource->width;
    uint32_t texture_height = texture_resource->height;
    
    float uv_x = ux / (float)texture_width;
    float uv_y = uy / (float)texture_height;
    float uv_w = uw / (float)texture_width;
    float uv_h = uh / (float)texture_height;

    _graphics_context_push_batch_quad(
        graphics_context,
        texture,
        (struct graphics_context_batch_quad) {
            .x = x,
            .y = y,
            .w = w,
            .h = h,

            .uv_x = uv_x,
            .uv_y = uv_y,
            .uv_w = uv_w,
            .uv_h = uv_h,

            .flags = flags,

            .rgba   = encode_rgba_from_float(r, g, b, a),
            .shader = shader,
        }
    );
}

void graphics_context_draw_textured_quad_ext_with_rotation(struct graphics_context* graphics_context, graphics_context_texture_handle texture, float x, float y, float w, float h, float ux, float uy, float uw, float uh, uint8_t flags, float rotation_pivot_x, float rotation_pivot_y, float degrees, float r, float g, float b, float a, graphics_context_shader_handle shader) {
    struct graphics_context_texture* texture_resource = &graphics_context->textures[texture.id];

    uint32_t texture_width  = texture_resource->width;
    uint32_t texture_height = texture_resource->height;
    
    float uv_x = ux / (float)texture_width;
    float uv_y = uy / (float)texture_height;
    float uv_w = uw / (float)texture_width;
    float uv_h = uh / (float)texture_height;

    _graphics_context_push_batch_quad(
        graphics_context,
        texture,
        (struct graphics_context_batch_quad) {
            .x = x,
            .y = y,
            .w = w,
            .h = h,

            .uv_x = uv_x,
            .uv_y = uv_y,
            .uv_w = uv_w,
            .uv_h = uv_h,

            .flags = flags,

            .rgba   = encode_rgba_from_float(r, g, b, a),
            .shader = shader,

            .rotation_pivot_x = rotation_pivot_x,
            .rotation_pivot_y = rotation_pivot_y,
            .rotation_degrees = degrees,
        }
    );
}

// NOTE(jerry):
// These do not screen-cull.
// These don't have to be as efficient
struct graphics_context_immediate_vertex {
    float x;
    float y;
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
};
static void _graphics_context_push_batch_immediate_vertices(struct graphics_context* graphics_context, struct graphics_context_immediate_vertex* immediate_vertices, size_t vertex_count, uint16_t* indices, size_t index_count) {
    bool should_flush = false;

    if (graphics_context->current_batch.texture.id != null_texture.id) {
        should_flush = true;
    }

    if (graphics_context->current_batch.shader.id != null_shader.id) {
        should_flush = true;
    }

    if (graphics_context->immediate_vertices_count >= SPACE_FOR_GENERIC_SHAPE_VERTICES) {
        should_flush = true;
    }

    if (should_flush) {
        _graphics_context_batch_flush_all_immediate_vertices(graphics_context);
        graphics_context->current_batch.texture = null_texture;
        graphics_context->current_batch.shader  = null_shader;
    }

    assertion(graphics_context->immediate_vertices_count + vertex_count < SPACE_FOR_GENERIC_SHAPE_VERTICES);

    // save current vertex count to offset indices later.
    uint16_t currently_used_vertices_at_the_time = graphics_context->immediate_vertices_count;

    for (size_t vertex_index = 0; vertex_index < vertex_count; ++vertex_index) {
        struct graphics_context_default_vertex_format* current_vertex = &graphics_context->batch_vertices[graphics_context->quad_batch_capacity*4 + graphics_context->immediate_vertices_count++];

        current_vertex->x = immediate_vertices[vertex_index].x;
        current_vertex->y = immediate_vertices[vertex_index].y;
        current_vertex->r = immediate_vertices[vertex_index].r;
        current_vertex->g = immediate_vertices[vertex_index].g;
        current_vertex->b = immediate_vertices[vertex_index].b;
        current_vertex->a = immediate_vertices[vertex_index].a;
    }

    // lol
    // offset indices, since the given indices are relative to their mesh.
    for (size_t index_index = 0; index_index < index_count; ++index_index) {
        graphics_context->batch_indices[graphics_context->quad_batch_capacity*6 + graphics_context->immediate_indices_count++] = indices[index_index] + currently_used_vertices_at_the_time;
    }
}

void graphics_context_draw_circle(struct graphics_context* graphics_context, float x, float y, float radius, float r, float g, float b, float a) {
    { // bounds checking
        float circle_min_x = x - radius;
        float circle_max_x = x + radius;
        float circle_min_y = y - radius;
        float circle_max_y = y + radius;

        bool x_overlap = circle_min_x < graphics_context->screen_bounds.right  && graphics_context->screen_bounds.left < circle_max_x;
        bool y_overlap = circle_min_y < graphics_context->screen_bounds.bottom && graphics_context->screen_bounds.top < circle_max_y;

        bool fits_in_screen = x_overlap && y_overlap;

        if (!fits_in_screen) {
            return;
        }
    }

    size_t current_index_index = 0;

    float  degree_granularity = 7.5f;
    size_t segment_count      = ((360.0f / degree_granularity)) + 1;

    float current_trace_angle = 0.0f;

    struct graphics_context_allocation_result allocation_for_vertices = graphics_context_push_buffer_allocate_forward(graphics_context, segment_count * sizeof(struct graphics_context_immediate_vertex));
    struct graphics_context_allocation_result allocation_for_indices  = graphics_context_push_buffer_allocate_forward(graphics_context, segment_count * sizeof(uint16_t) * 3);

    struct graphics_context_immediate_vertex* circle_vertices = allocation_for_vertices.memory;
    uint16_t*                                 circle_indices  = allocation_for_indices.memory;

    // center point
    {
        circle_vertices[0].x = x;
        circle_vertices[0].y = y;

        circle_vertices[0].r = (uint8_t)(r * 255.0f);
        circle_vertices[0].g = (uint8_t)(g * 255.0f);
        circle_vertices[0].b = (uint8_t)(b * 255.0f);
        circle_vertices[0].a = (uint8_t)(a * 255.0f);
    }

    // initial point... Unrolled to avoid special casing.
    {
        struct vector2 point_on_circumference = vector2(cosf(degree_to_radians(current_trace_angle)) * radius + x, sinf(degree_to_radians(current_trace_angle)) * radius + y);

        circle_vertices[1].x = point_on_circumference.x;
        circle_vertices[1].y = point_on_circumference.y;

        circle_vertices[1].r = (uint8_t)(r * 255.0f);
        circle_vertices[1].g = (uint8_t)(g * 255.0f);
        circle_vertices[1].b = (uint8_t)(b * 255.0f);
        circle_vertices[1].a = (uint8_t)(a * 255.0f);

        current_trace_angle += degree_granularity;

        circle_indices[current_index_index++] = 0;               // center point
        circle_indices[current_index_index++] = segment_count-1; // last    vertex
        circle_indices[current_index_index++] = 1;   // current vertex

        current_trace_angle += degree_granularity;
    }

    for (size_t segment_index = 1; segment_index < segment_count; ++segment_index) {
        struct vector2 point_on_circumference = vector2(cosf(degree_to_radians(current_trace_angle)) * radius + x, sinf(degree_to_radians(current_trace_angle)) * radius + y);

        circle_vertices[segment_index].x = point_on_circumference.x;
        circle_vertices[segment_index].y = point_on_circumference.y;

        circle_vertices[segment_index].r = (uint8_t)(r * 255.0f);
        circle_vertices[segment_index].g = (uint8_t)(g * 255.0f);
        circle_vertices[segment_index].b = (uint8_t)(b * 255.0f);
        circle_vertices[segment_index].a = (uint8_t)(a * 255.0f);

        current_trace_angle += degree_granularity;

        circle_indices[current_index_index++] = 0;               // center point
        circle_indices[current_index_index++] = segment_index-1; // previous vertex
        circle_indices[current_index_index++] = segment_index;   // current vertex
    }

    _graphics_context_push_batch_immediate_vertices(graphics_context, circle_vertices, segment_count, circle_indices, current_index_index);

    graphics_context_push_buffer_restore(graphics_context, allocation_for_indices.restoration_marker);
    graphics_context_push_buffer_restore(graphics_context, allocation_for_vertices.restoration_marker);
}
void graphics_context_draw_line(struct graphics_context* graphics_context, float x1, float y1, float x2, float y2, float thickness, float r, float g, float b, float a) {
    // NOTE(jerry):
    // this isn't even technically a correct bounding box
    // but it's close enough and won't be noticable.
    { // bounds checking
        float line_bounding_rectangle_min_x = (x1)      - thickness;
        float line_bounding_rectangle_max_x = (x2 - x1) + thickness;
        float line_bounding_rectangle_min_y = (y1)      - thickness;
        float line_bounding_rectangle_max_y = (y2 - y1) + thickness;

        bool x_overlap = line_bounding_rectangle_min_x < graphics_context->screen_bounds.right  && graphics_context->screen_bounds.left < line_bounding_rectangle_max_x;
        bool y_overlap = line_bounding_rectangle_min_y < graphics_context->screen_bounds.bottom && graphics_context->screen_bounds.top  < line_bounding_rectangle_max_y;

        bool fits_in_screen = x_overlap && y_overlap;

        if (!fits_in_screen) {
            return;
        }
    }

    struct vector2 line_normal = vector2_perpendicular(vector2_direction_between(vector2(x1, y1), vector2(x2, y2)));
    float half_thickness = thickness / 2.0;

    uint8_t r_u8 = (uint8_t)(r * 255.0f);
    uint8_t g_u8 = (uint8_t)(g * 255.0f);
    uint8_t b_u8 = (uint8_t)(b * 255.0f);
    uint8_t a_u8 = (uint8_t)(a * 255.0f);

    struct graphics_context_immediate_vertex line_vertices[4] = {
        {x1 + line_normal.x * half_thickness, y1 + line_normal.y * half_thickness, r_u8, g_u8, b_u8, a_u8},
        {x1 - line_normal.x * half_thickness, y1 - line_normal.y * half_thickness, r_u8, g_u8, b_u8, a_u8},
        {x2 + line_normal.x * half_thickness, y2 + line_normal.y * half_thickness, r_u8, g_u8, b_u8, a_u8},
        {x2 - line_normal.x * half_thickness, y2 - line_normal.y * half_thickness, r_u8, g_u8, b_u8, a_u8},
    };
    uint16_t line_indices[6] = {
        0, 1, 2, 2, 1, 3
    };

    _graphics_context_push_batch_immediate_vertices(graphics_context, line_vertices, 4, line_indices, 6); 
}
void graphics_context_draw_rectangle(struct graphics_context* graphics_context, float x, float y, float w, float h, float rotation_degrees, float r, float g, float b, float a) {
    graphics_context_draw_untextured_quad_with_rotation(graphics_context, x, y, w, h, 0.5, 0.5, rotation_degrees, r, g, b, a, null_shader);
}

// This glyph cache is not optimally fast, and infact is probably kind of slow
// it's only saving grace is that it fits within a fixed memory footprint.
// It works fine if text remains the same across frames and we don't have many different characters.
// Basically... If you can somehow fit more than 2048 characters on a screen, we might be in trouble.
// Of course that's assuming we have no hash collisions. Which the cache will straight up replace. Which can
// be harsh.
struct graphics_context_font_glyph* _graphics_context_fetch_or_cache_codepoint(struct graphics_context* graphics_context, struct graphics_context_font_handle font, uint32_t codepoint) {
    // ASCII codepoints don't have to be cached.
    struct graphics_context_font* font_resource = &graphics_context->fonts[font.id];
    if (codepoint <= 128) {
        return &font_resource->glyphs[codepoint];
    }

    // NOTE(jerry):
    // Please find a better hash function that avoids collisions!
    // this is critical to improving the performance of this thing because
    // pruning takes more time than a direct jump.

    // Even if those upper bits are empty, we can still get a better hash because the multiplication
    // and xor in fnv will add more information to make a better hash output.
    uint64_t codepoint_as_64bit = codepoint - 128;

    // 128 bit hash should help alleviate more collisions
    char hash_bytes[128] = {};
    memcpy(hash_bytes, font_resource->file_path, 63);
    memcpy(hash_bytes+64, &font_resource->font_size, sizeof(float));
    memcpy(hash_bytes+64+sizeof(float), &codepoint_as_64bit, sizeof(uint64_t));

    uint64_t hash_key = fnv1a_hash(hash_bytes, 128);
    // NOTE(jerry):
    // 1000 -> 8
    //   (minus one) 0111 -> 7. X MOD 8 will only produce numbers from [0, 7],
    //                          which happens to be equal to ANDing by the bits of 7, or 8-1
    // This only works on powers of two.
    uint64_t hash_index = hash_key & (graphics_context->spare_glyph_cache_capacity - 1);

    struct graphics_context_font_glyph* glyph = &graphics_context->spare_glyph_cache[hash_index];

    if (codepoint != glyph->codepoint || graphics_context->glyph_owners[hash_index] != font.id) {
        // linear probe to prune for remaining slots.
        // This is kind of retarded but it's better than not handling collisions at all.
        // I'm not chaining these. As I need to deallocate these.
        while (hash_index < graphics_context->spare_glyph_cache_capacity) {
            glyph = &graphics_context->spare_glyph_cache[hash_index];

            if (glyph->texture_handle.id == null_texture.id) {
                break; 
            }

            if (hash_index + 1 >= graphics_context->spare_glyph_cache_capacity) {
                hash_index = 0;
                break;
            }

            hash_index += 1;
        }

        if (glyph->texture_handle.id != null_texture.id) {
            graphics_context_unload_texture(graphics_context, glyph->texture_handle);
            glyph->texture_handle = null_texture;
        }

        graphics_context->glyph_owners[hash_index] = font.id;

        stbtt_fontinfo font_info;
        uint8_t* file_buffer = font_resource->file_buffer;
        stbtt_InitFont(&font_info, file_buffer, 0);

        float scale_factor = stbtt_ScaleForPixelHeight(&font_info, font_resource->font_size);
        _graphics_context_font_load_fill_glyph(graphics_context, glyph, codepoint, &font_info, scale_factor);
    }

    return glyph;
}
// baseline is not at top left corner like you might expect!
// it's at the bottom! To readjust just add ascent and font_size
void graphics_context_draw_codepoint(struct graphics_context* graphics_context, graphics_context_font_handle font, float x, float y, uint32_t codepoint, float font_scale, float r, float g, float b, float a) {
    struct graphics_context_font*       font_resource   = &graphics_context->fonts[font.id];
    struct graphics_context_font_glyph* codepoint_glyph = _graphics_context_fetch_or_cache_codepoint(graphics_context, font, codepoint);

    float scale = font_scale;
    float left_bearing = codepoint_glyph->left_side_bearing * scale;

    float width         = (codepoint_glyph->bitmap_right - codepoint_glyph->bitmap_left) * scale;
    float height        = (codepoint_glyph->bitmap_bottom - codepoint_glyph->bitmap_top) * scale;
    float bitmap_bottom = (codepoint_glyph->bitmap_bottom) * scale;

    float bitmap_power_of_two_size = codepoint_glyph->_power_of_two_size * scale;
    // this is to set a close enough to correct baseline.
    {
        float uv_left   = 0;
        float uv_top    = 0;
        float uv_right  = width / bitmap_power_of_two_size;
        float uv_bottom = height / bitmap_power_of_two_size;

        bool is_ascii = codepoint <= 127 && codepoint >= 0;

        // This little exception is made as ASCII fonts are the only fonts that are currently atlased!
        if (is_ascii) {
            uv_left   = codepoint_glyph->texture_coordinate_x;
            uv_top    = codepoint_glyph->texture_coordinate_y;
            uv_right  = uv_left + codepoint_glyph->texture_coordinate_w;
            uv_bottom = uv_top  + codepoint_glyph->texture_coordinate_h;

            uv_left   /= bitmap_power_of_two_size;
            uv_right  /= bitmap_power_of_two_size;
            uv_top    /= bitmap_power_of_two_size;
            uv_bottom /= bitmap_power_of_two_size;
        }

        _graphics_context_push_batch_quad(
            graphics_context,
            codepoint_glyph->texture_handle,
            (struct graphics_context_batch_quad) {
                .x = x + left_bearing,
                .y = y + (bitmap_bottom - height),
                .w = width,
                .h = height,

                .uv_x = uv_left,
                .uv_y = uv_top,
                .uv_w = uv_right - uv_left,
                .uv_h = uv_bottom - uv_top,

                .rgba   = encode_rgba_from_float(r, g, b, a),
                .shader = graphics_context->text_shader_program,
            }
        );

        if (!is_ascii) {
            _graphics_context_flush_all(graphics_context);
        }
    }
}

void graphics_context_draw_text(struct graphics_context* graphics_context, graphics_context_font_handle font, float x, float y, char* text_utf8, float font_scale, float r, float g, float b, float a) {
    struct graphics_context_font* font_resource = &graphics_context->fonts[font.id];
    float scale_factor = font_scale;

    float x_cursor = x;
    float y_cursor = y + (font_resource->font_size) * scale_factor; // NOTE(jerry): resets baseline to look correct.

    for (decode_utf8_iterator iterator = decode_utf8_from_cstring(text_utf8); decode_utf8_iterator_valid(&iterator); decode_utf8_iterator_advance(&iterator)) {
        uint32_t codepoint = iterator.codepoint;
        struct graphics_context_font_glyph* glyph = _graphics_context_fetch_or_cache_codepoint(graphics_context, font, codepoint);

        if (codepoint != ' ' && is_whitespace_character(codepoint)) {
            switch (codepoint) {
                case '\r':
                case '\n': {
                    x_cursor = x;
                    y_cursor += (font_resource->font_size) * scale_factor;
                } break;
                case '\t': {
                    x_cursor += (glyph->advance_width) * scale_factor * 4;
                } break;
            }
        } else {
            graphics_context_draw_codepoint(graphics_context, font, x_cursor, y_cursor, codepoint, font_scale, r, g, b, a);
            x_cursor += glyph->advance_width * scale_factor;
        }
    }
}

struct graphics_context_text_extents graphics_context_measure_text(struct graphics_context* graphics_context, graphics_context_font_handle font, char* text_utf8, float font_scale) {
    struct graphics_context_text_extents measurements = {};
    struct graphics_context_font* font_resource = &graphics_context->fonts[font.id];

    char* cursor = text_utf8;
    float maximum_width = 0;
    float x_cursor = 0;
    float y_cursor = (font_resource->font_size) * font_scale;

    // NOTE(jerry): Does not use iterator. Fix it when you feel like it.
    while ((*cursor) != '\0') {
        struct utf8_decode_result decode_packet = utf8_decode_single_codepoint(cursor);
        cursor += (decode_packet.decoded_bytes);

        uint32_t codepoint = decode_packet.codepoint;
        struct graphics_context_font_glyph* glyph = &font_resource->glyphs[codepoint];

        if (codepoint != ' ' && is_whitespace_character(codepoint)) {
            switch (codepoint) {
                case '\r':
                case '\n': {
                    x_cursor = 0;
                    y_cursor += (font_resource->font_size) * font_scale;
                } break;
                case '\t': {
                    x_cursor += (glyph->advance_width) * font_scale * 4;
                } break;
            }
        } else {
            x_cursor += glyph->advance_width * font_scale;

            if (x_cursor > maximum_width) {
                maximum_width = x_cursor;
            }
        }
    }

    measurements.width  = maximum_width;
    measurements.height = y_cursor;

    return measurements;
}

// TODO(jerry):
// these handles we do actually want to do safety checks.
// I just don't do them yet!
struct graphics_context_texture_information graphics_context_texture_information(struct graphics_context* graphics_context, graphics_context_texture_handle texture) {
    struct graphics_context_texture* texture_resource = &graphics_context->textures[texture.id];
    struct graphics_context_texture_information result = {};

    strncpy(texture_resource->file_path, result.file_path, BLACKIRON_MAX_PLATFORM_PATHNAME_LENGTH);
    result.width  = texture_resource->width;
    result.height = texture_resource->height;

    return result;
}

struct graphics_context_font* graphics_context_dereference_font(struct graphics_context* graphics_context, graphics_context_font_handle font) {
    struct graphics_context_font* font_resource = &graphics_context->fonts[font.id];
    return font_resource;
}

// TODO(jerry): While I do free the resources, I never actually
// invalidate the handle. So do that later!
// (none of the handles actually do validation right now :/)
static void _graphics_context_unload_texture_resource(struct graphics_context* graphics_context, uint16_t texture_index) {
    struct graphics_context_texture* texture_resource = &graphics_context->textures[texture_index];
    glDeleteTextures(1, &texture_resource->texture_handle);
    texture_resource->texture_handle = 0;
    texture_resource->status = GRAPHICS_CONTEXT_RESOURCE_STATUS_UNLOADED;

    /* if (cstring_length(texture_resource->file_path)) { */
    if (cstring_length(texture_resource->file_path)) {
        file_hentai_stop_stalking_file(texture_resource->file_path);
        memset(texture_resource->file_path, 0, BLACKIRON_MAX_PLATFORM_PATHNAME_LENGTH);
    }
}
void graphics_context_unload_texture(struct graphics_context* graphics_context, graphics_context_texture_handle texture) {
    if (texture.id == null_texture.id) {
        return;
    }

    _graphics_context_unload_texture_resource(graphics_context, texture.id);
}

static void _graphics_context_unload_font_resource(struct graphics_context* graphics_context, uint16_t font_index) {
    struct graphics_context_font* font_resource = &graphics_context->fonts[font_index];
    for (unsigned glyph_index = 0; glyph_index < 128; ++glyph_index) {
        struct graphics_context_font_glyph* glyph = &font_resource->glyphs[glyph_index];
        graphics_context_unload_texture(graphics_context, glyph->texture_handle);
        glyph->texture_handle = null_texture;
    }
    font_resource->status = GRAPHICS_CONTEXT_RESOURCE_STATUS_UNLOADED;
}
void graphics_context_unload_font(struct graphics_context* graphics_context, graphics_context_font_handle font) {
    _graphics_context_unload_font_resource(graphics_context, font.id);

    // NOTE(jerry):
    // Consider just biting the bullet and making a linked list of file buffers so I can just free them trivially.

    graphics_context->memory_used_top = 0;

    // Reload all the fonts into the top of the stack.
    for (unsigned font_index = 0; font_index < graphics_context->font_capacity; ++font_index) {
        struct graphics_context_font* current_font_resource = &graphics_context->fonts[font_index];

        if (current_font_resource->file_buffer) {
            void* new_file_buffer = _graphics_context_font_resources_attempt_to_reuse_existing_file_buffer(graphics_context, current_font_resource->file_buffer);
            if (new_file_buffer == NULL) {
                new_file_buffer = _graphics_context_read_file_into_buffer(graphics_context, current_font_resource->file_path);
            }

            current_font_resource->file_buffer = new_file_buffer;
        }
    }
}
static void _graphics_context_unload_shader_resource(struct graphics_context* graphics_context, uint16_t shader_index) {
    struct graphics_context_shader* shader_resource = &graphics_context->shaders[shader_index];

    if (shader_resource->status == GRAPHICS_CONTEXT_RESOURCE_STATUS_READY) {
        shader_resource->status    = GRAPHICS_CONTEXT_RESOURCE_STATUS_UNLOADED;
        shader_resource->from_file = false;
        shader_resource->program   = 0;
        memset(shader_resource->vertex_file_path, 0, BLACKIRON_MAX_PLATFORM_PATHNAME_LENGTH);
        memset(shader_resource->fragment_file_path, 0, BLACKIRON_MAX_PLATFORM_PATHNAME_LENGTH);
        glDeleteProgram(shader_resource->program);
    }
}
void graphics_context_unload_shader(struct graphics_context* graphics_context, graphics_context_shader_handle shader) {
    _graphics_context_unload_shader_resource(graphics_context, shader.id);
}

// assume that x and y are in normal window space
void graphics_context_map_screenspace_point_into_virtual_resolution(struct graphics_context* graphics_context, float* x, float* y) {
    struct graphics_context_viewport_rectangle viewport = _viewport_rectangle_for_virtual_resolution(graphics_context->screen_width, graphics_context->screen_height, graphics_context->virtual_resolution.width, graphics_context->virtual_resolution.height);

    // Undo translation to account for the padding done by letterboxing or pillar boxing.
    // Scale factor is from virtual -> real, so the inverse of that is real -> virtual, so divide.
    // The boxing padding is done based off the delta of the original screen dimension and the scaled virtual dimensions
    // so the translation must be done first.
    if (x) {
        (*x) -= viewport.x;
        (*x) /= viewport.scale_factor;
    }

    if (y) {
        (*y) -= viewport.y;
        (*y) /= viewport.scale_factor;
    }
}


// if they are "screen" render targets we can resize them.
// Now we actually want to resize based off the virtual resolution
// since that's technically the final product.
// (Or not? Maybe I should resample it???)
static void graphics_context_resize_all_applicable_render_targets(struct graphics_context* graphics_context) {
    int32_t target_resolution_width = (int32_t)graphics_context->virtual_resolution.width;
    if ((int)graphics_context->virtual_resolution.width == 0) {
        target_resolution_width = graphics_context->screen_width;
    }
    int32_t target_resolution_height = (int32_t)graphics_context->virtual_resolution.height;
    if ((int)graphics_context->virtual_resolution.height == 0) {
        target_resolution_height = graphics_context->screen_height;
    }

    // render targets will never be at slot 1.
    for (size_t index = 1; index < graphics_context->texture_capacity; ++index) {
        struct graphics_context_texture* texture_resource = &graphics_context->textures[index];

        if (texture_resource->is_render_target) {
            if (texture_resource->width == target_resolution_width && texture_resource->height == target_resolution_height) {
                continue;
            } else {
                // Depends on the driver me thinks?
                // might be expensive...
                // If it's bad just resize the original texture then...
                GLuint framebuffer_object;
                GLuint framebuffer_color_attachment;

                {
                    glGenFramebuffers(1, &framebuffer_object);
                    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_object);

                    glGenTextures(1, &framebuffer_color_attachment);
                    glBindTexture(GL_TEXTURE_2D, framebuffer_color_attachment);

                    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, target_resolution_width, target_resolution_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

                    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebuffer_color_attachment, 0);
                    glBindFramebuffer(GL_FRAMEBUFFER, 0);
                }

                {
                    GLuint old_framebuffer    = texture_resource->framebuffer_handle;
                    GLuint old_texture_handle = texture_resource->texture_handle;
                    glDeleteFramebuffers(1, &old_framebuffer);
                    glDeleteTextures(1, &old_texture_handle);
                }

                texture_resource->framebuffer_handle = framebuffer_object;
                texture_resource->texture_handle     = framebuffer_color_attachment;

                texture_resource->width              = target_resolution_width;
                texture_resource->height             = target_resolution_height;

            }
        }
    }
}

void graphics_context_set_virtual_resolution(struct graphics_context* graphics_context, float width, float height) {
    graphics_context->virtual_resolution.width = width;
    graphics_context->virtual_resolution.height = height;

    graphics_context_resize_all_applicable_render_targets(graphics_context);
}

void graphics_context_update_screen_dimensions(struct graphics_context* graphics_context, int32_t new_width, int32_t new_height) {
    if (graphics_context->screen_width != new_width || graphics_context->screen_height != new_height) {
        graphics_context->screen_width  = new_width;
        graphics_context->screen_height = new_height;

        graphics_context_resize_all_applicable_render_targets(graphics_context);
    }
}

void _graphics_context_reload_texture(struct graphics_context* graphics_context, graphics_context_texture_handle texture) {
    struct graphics_context_texture* texture_resource = &graphics_context->textures[texture.id];

    if (texture_resource->status == GRAPHICS_CONTEXT_RESOURCE_STATUS_READY) {
        char* texture_file_path = texture_resource->file_path;
            
        struct __texture_loading_result result = _graphics_context_handle_loading_texture_from_file_and_padding_to_power_of_two(graphics_context, texture_file_path);
            
        texture_resource->texture_handle     = result.texture_handle;
        texture_resource->width              = result.width;
        texture_resource->height             = result.height;
        texture_resource->_power_of_two_size = result.padded_size;
    }
}

void _graphics_context_reload_shader(struct graphics_context* graphics_context, graphics_context_shader_handle shader) {
    struct graphics_context_shader* shader_resource = &graphics_context->shaders[shader.id];

    if (shader_resource->status == GRAPHICS_CONTEXT_RESOURCE_STATUS_READY && shader_resource->from_file) {
        char* vertex_file_path   = shader_resource->vertex_file_path;
        if (cstring_length(shader_resource->vertex_file_path) == 0) {
            vertex_file_path = NULL;
        }

        char* fragment_file_path = shader_resource->fragment_file_path;
        if (cstring_length(shader_resource->fragment_file_path) == 0) {
            fragment_file_path = NULL;
        }

        struct graphics_context_shader_result shader_creation = _graphics_context_handle_shader_construction_from_files(vertex_file_path, fragment_file_path);

        if (shader_creation.error) {
            console_printf("RELOAD ERROR!\n");
            console_printf("Vertex Error Log:\n%.*s\n",   GRAPHICS_CONTEXT_MAX_SHADER_LOG_LENGTH, shader_creation.vertex_error_log);
            console_printf("Fragment Error Log:\n%.*s\n", GRAPHICS_CONTEXT_MAX_SHADER_LOG_LENGTH, shader_creation.fragment_error_log);
            console_printf("Linkage Error Log:\n%.*s\n",  GRAPHICS_CONTEXT_MAX_SHADER_LOG_LENGTH, shader_creation.linkage_error_log);
        } else {
            shader_resource->program = shader_creation.program;
            console_printf("Shader was successfully reloaded from a file! (prg id: %d)\n", shader_creation.program);

            // to keep all shaders in sync, we have to upload the projection matrix so we can do the right vertex
            // transforms and have stuff look correct.
            _graphics_context_upload_matrix_uniform_to_all_shader_programs(graphics_context, "projection_matrix", graphics_context->matrices.projection);
        }
    }
}

void _graphics_context_reload_resource(char* file_name, void* user_data) {
    struct graphics_context_file_hentai_user_data_packet* user_data_packet = user_data;

    struct graphics_context* graphics_context = user_data_packet->context;

    // NOTE(jerry):
    // Admittedly... Now I have to pay attention to the security of these handles!
    // Shaders will easily break because they don't get hashed properly!
    // Textures are fine however because they're hashed and their slots remain the same
    // through unloads. But shaders will actively switch positions which is bad.
    // I can build a handle->index lookup table for shaders which would basically just make
    // them like a hashtable, and that may work fine as well.

    switch (user_data_packet->resource_type) {
        case GRAPHICS_CONTEXT_RESOURCE_TYPE_TEXTURE: {
            _graphics_context_reload_texture(graphics_context, user_data_packet->texture);
        } break;
        case GRAPHICS_CONTEXT_RESOURCE_TYPE_SHADER: {
            _graphics_context_reload_shader(graphics_context, user_data_packet->shader);
        } break;
    }
}

void graphics_context_reload_all_shaders(struct graphics_context* graphics_context) {
    console_printf("Reloading all shaders...\n");

    // these are the only things that obey the 0 is reserved rule thus far!
    for (size_t shader_index = 1; shader_index <= graphics_context->shader_capacity; ++shader_index) {
        struct graphics_context_shader* shader_resource = &graphics_context->shaders[shader_index];

        if (shader_resource->status == GRAPHICS_CONTEXT_RESOURCE_STATUS_UNLOADED && shader_resource->from_file) {
            char* vertex_file_path   = shader_resource->vertex_file_path;
            if (cstring_length(shader_resource->vertex_file_path) == 0) {
                vertex_file_path = NULL;
            }

            char* fragment_file_path = shader_resource->fragment_file_path;
            if (cstring_length(shader_resource->fragment_file_path) == 0) {
                fragment_file_path = NULL;
            }

            struct graphics_context_shader_result shader_creation = _graphics_context_handle_shader_construction_from_files(vertex_file_path, fragment_file_path);

            if (shader_creation.error) {
                console_printf("RELOAD ERROR!\n");
                console_printf("Vertex Error Log:\n%.*s\n",   GRAPHICS_CONTEXT_MAX_SHADER_LOG_LENGTH, shader_creation.vertex_error_log);
                console_printf("Fragment Error Log:\n%.*s\n", GRAPHICS_CONTEXT_MAX_SHADER_LOG_LENGTH, shader_creation.fragment_error_log);
                console_printf("Linkage Error Log:\n%.*s\n",  GRAPHICS_CONTEXT_MAX_SHADER_LOG_LENGTH, shader_creation.linkage_error_log);
            } else {
                shader_resource->program = shader_creation.program;
                console_printf("Shader was successfully reloaded from a file! (prg id: %d)\n", shader_creation.program);
            }
        }
    }
}

void graphics_context_reload_all_textures(struct graphics_context* graphics_context) {
    console_printf("Reloading all textures...\n");
    // If I stored the keys
    // I could iterate them more conveniently but whatever

    for (size_t texture_index = 0; texture_index < graphics_context->texture_capacity; ++texture_index) {
        struct graphics_context_texture* texture_resource = &graphics_context->textures[texture_index];

        if (texture_resource->status == GRAPHICS_CONTEXT_RESOURCE_STATUS_READY) {
            char* texture_file_path = texture_resource->file_path;
            
            struct __texture_loading_result result = _graphics_context_handle_loading_texture_from_file_and_padding_to_power_of_two(graphics_context, texture_file_path);
            
            texture_resource->texture_handle     = result.texture_handle;
            texture_resource->width              = result.width;
            texture_resource->height             = result.height;
            texture_resource->_power_of_two_size = result.padded_size;
        }
    }
}

void graphics_context_reload_all_fonts(struct graphics_context* graphics_context) {
    console_printf("Fonts cannot be hotreloaded! :(\n");
}

void graphics_context_reload_all_resources(struct graphics_context* graphics_context) {
    graphics_context_reload_all_shaders(graphics_context);
    graphics_context_reload_all_textures(graphics_context);
    graphics_context_reload_all_fonts(graphics_context);
}

void graphics_context_update(struct graphics_context* graphics_context, float delta_time) {
    graphics_context->elapsed_time += delta_time;

    for (size_t shader_index = 1; shader_index <= graphics_context->shader_count; ++shader_index) {
        _opengl_shader_uniform_set_float1(graphics_context->shaders[shader_index].program, "elapsed_time", graphics_context->elapsed_time);
    }
}

void graphics_context_deinitialize(struct graphics_context* graphics_context) {
    for (size_t texture_index = 0; texture_index < graphics_context->texture_capacity; ++texture_index) {
        _graphics_context_unload_texture_resource(graphics_context, texture_index);
    }

    for (size_t font_index = 0; font_index < graphics_context->font_capacity; ++font_index) {
        _graphics_context_unload_font_resource(graphics_context, font_index);
    }

    for (size_t shader_index = 0; shader_index < graphics_context->shader_capacity; ++shader_index) {
        _graphics_context_unload_shader_resource(graphics_context, shader_index);
    }

    system_memory_deallocate(graphics_context->memory);
}
