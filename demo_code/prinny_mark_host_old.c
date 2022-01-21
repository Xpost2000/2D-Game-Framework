/*
  PRINNYMARK:
  a basic sprite rendering demo, to see how many sprites I can render while still being performant.
  
  Has basic dynamic lighting with all sprites that are visible on the screen, updated and doing basic logic enmasse.
  These sprites do not collide with each other since that's not really a rendering demo, just a physics demo.
  
  All the sprites are animated separately and have different animation ids to hit the worth case.
  
  I can hit about 30FPS on 500000 sprites if they all use the same texture ID (so they are batched optimally),

  once we add sprite atlases, prinny mark will be invincible!
  
  I have no idea if the engine should be able to build it's own sprite atlases, since that API can be a bit tricky and
  opinionated, so I'll just settle for being able to render based on uv coordinates.
  
  Then we shall see the power of PRINNYMARK!
*/
#include "public/common.h"
#include "public/configuration.h"

#include "public/graphics_context_api.h"
#include "public/system_api.h"
#include "public/input_api.h"

#include "public/host_state.h"

#include "public/vector2d.h"
#include "public/collision.h"

#define system _std_system
#include <stdlib.h>
#undef system

graphics_context_shader_handle grayscale_shader;

size_t prinny_count = 0;
struct prinny_particle* prinnies;

static struct graphics_context_api* graphics;
static struct input_api*            input;
static struct system_api*           system;

graphics_context_font_handle    arial_font;
graphics_context_font_handle    alkhemikal_font;
graphics_context_font_handle    jp_font;
graphics_context_font_handle    smaller_arial_font;

graphics_context_texture_handle prinny_sheet;

graphics_context_texture_handle light_blob;
graphics_context_texture_handle test_tile;

enum prinny_animation_id {
    PRINNY_ANIMATION_ID_IDLE,
    PRINNY_ANIMATION_ID_SPINNING,
    PRINNY_ANIMATION_ID_YELP,
    PRINNY_ANIMATION_ID_COUNT=2,
};
struct uv_rectangle {
    uint16_t x;
    uint16_t y;
    uint16_t w;
    uint16_t h;
};

// montage image
// 512 / 4 = 128
// 504 / 4 = 126

static struct uv_rectangle idle_rects[1] = {
    (struct uv_rectangle) {0, 0, 128, 126}
};
static struct uv_rectangle yelp_rects[1] = {
    (struct uv_rectangle) {3*128, 3*126, 128, 126}
};
//obviously in real games please have tools generate this.
// this is a pain to do by hand lol.
static struct uv_rectangle spinning_rects[14] = {
    (struct uv_rectangle) {1*128,     0, 128, 126},
    (struct uv_rectangle) {2*128,     0, 128, 126},
    (struct uv_rectangle) {0,     1*126, 128, 126},
    (struct uv_rectangle) {2*128, 1*126, 128, 126},
    (struct uv_rectangle) {0,     2*126, 128, 126},
    (struct uv_rectangle) {2*128, 2*126, 128, 126},
    (struct uv_rectangle) {0,     3*126, 128, 126},
    (struct uv_rectangle) {2*128, 3*126, 128, 126},
    (struct uv_rectangle) {1*128, 3*126, 128, 126},
    (struct uv_rectangle) {3*128, 2*126, 128, 126},
    (struct uv_rectangle) {1*128, 2*126, 128, 126},
    (struct uv_rectangle) {3*128, 1*126, 128, 126},
    (struct uv_rectangle) {1*128, 1*126, 128, 126},
    (struct uv_rectangle) {3*128,     0, 128, 126},
};

struct prinny_animation {
    uint8_t is_spritesheet;

    uint16_t count;
    union {
        graphics_context_texture_handle* textures;
        struct {
            graphics_context_texture_handle  atlas_texture;
            struct uv_rectangle*     subsprites;
        };
    };
    float time_until_next_frame;
} animations[16] = {
    [PRINNY_ANIMATION_ID_IDLE] = (struct prinny_animation) {
        .count = 1,
        .is_spritesheet = 1,
        .subsprites = idle_rects,
        /* .textures = prinny_idle */
    },

    [PRINNY_ANIMATION_ID_SPINNING] = (struct prinny_animation) {
        .count = 14,
        .is_spritesheet = 1,
        .subsprites = spinning_rects,
        /* .textures = prinny_spinning, */
        .time_until_next_frame = 0.05,
    },

    [PRINNY_ANIMATION_ID_YELP] = (struct prinny_animation) {
        .count = 1,
        .is_spritesheet = 1,
        .subsprites = yelp_rects,
        /* .textures = prinny_yelp, */
    },
};

struct prinny_particle {
    uint16_t animation_id;
    int16_t speed;

    uint8_t frame_index;
    bool going_forward;

    float x;
    float y;

    float frame_timer;
};

static void prinny_particles_update(int screen_width, int screen_height, float delta_time, struct prinny_particle* prinnies, size_t count) {
    for (size_t prinny_index = 0; prinny_index < count; ++prinny_index) {
        struct prinny_particle* current_prinny = &prinnies[prinny_index];
        struct prinny_animation* animation = &animations[current_prinny->animation_id];

        current_prinny->frame_timer += delta_time;

        /* if (current_prinny->animation_id != PRINNY_ANIMATION_ID_IDLE) { */
        if (current_prinny->x <= 0) {
            current_prinny->going_forward ^= 1;
            current_prinny->x = 1;
        } else if (current_prinny->x + 90 >= (screen_width*1.5)) {
            current_prinny->going_forward ^= 1;
            current_prinny->x = (screen_width * 1.48) - 90;
        }

        if (current_prinny->going_forward) {
            current_prinny->x += current_prinny->speed * delta_time;
        } else {
            current_prinny->x -= current_prinny->speed * delta_time;
        }
        /* } */

        if (current_prinny->frame_timer >= animation->time_until_next_frame) {
            current_prinny->frame_timer = 0;
            current_prinny->frame_index += 1;
        }

        if (current_prinny->frame_index >= animation->count) {
            current_prinny->frame_index = 0;
        }
    }
}
struct light {
    float x;
    float y;

    float r;
    float g;
    float b;

    float power;
};

size_t light_count;
struct light lights[256];
void light_push(float x, float y, float power, float r, float g, float b) {
    struct light* new_light = &lights[light_count++];
    new_light->x = x;
    new_light->y = y;
    new_light->power = power;
    new_light->r = r;
    new_light->g = g;
    new_light->b = b;
}

struct color {
    float r;
    float g;
    float b;
};
struct color calculate_basic_lighting(float x, float y, struct light* lights, size_t light_count) {
    struct color result = {0.22, 0.22, 0.22};

    for (size_t light_index = 0; light_index < light_count; ++light_index) {
        struct light* current_light = &lights[light_index];
        float distance_from_light = vector2_distance_between(
            (struct vector2) { x, y },
            (struct vector2) { current_light->x, current_light->y }
        );

        result.r += current_light->r*(1.0 / (1.0 + 2.5*distance_from_light)) * current_light->power;
        result.g += current_light->g*(1.0 / (1.0 + 2.5*distance_from_light)) * current_light->power;
        result.b += current_light->b*(1.0 / (1.0 + 2.5*distance_from_light)) * current_light->power;
    }

    return result;
}

static void prinny_particles_render(struct prinny_particle* prinnies, size_t count) {
    for (size_t prinny_index = 0; prinny_index < count; ++prinny_index) {
        struct prinny_particle* current_prinny = &prinnies[prinny_index];
        struct prinny_animation* animation = &animations[current_prinny->animation_id];

        struct color brightness = calculate_basic_lighting(current_prinny->x, current_prinny->y, lights, light_count);

        graphics_context_shader_handle shader = null_shader;

        /* if ((prinny_index % 4) == 0) shader = grayscale_shader; */

        if (animation->is_spritesheet) {
            int w = 128/4;
            int h = 126/4;
            graphics_context_texture_handle texture_handle = animation->atlas_texture;
            struct uv_rectangle texcoords = animation->subsprites[current_prinny->frame_index];

            graphics->draw_textured_quad_ext(texture_handle, current_prinny->x - w/2, current_prinny->y - h/2, w, h, texcoords.x, texcoords.y, texcoords.w, texcoords.h, 0, brightness.r, brightness.g, brightness.b, 1.0, shader);
        } else {
            int w = 96/3;
            int h = 135/3;
            graphics_context_texture_handle texture_handle = animation->textures[current_prinny->frame_index];
            graphics->draw_textured_quad(texture_handle, current_prinny->x - w/2, current_prinny->y - h/2, w, h, brightness.r, brightness.g, brightness.b, 1.0, shader);
        }
    }
}


void push_random_prinny(void) {
    struct prinny_particle* current_prinny = &prinnies[prinny_count++];
    current_prinny->x = rand() % (64 * 50);
    current_prinny->y = rand() % (30 * 50);

    uint16_t animation = rand() % PRINNY_ANIMATION_ID_COUNT;
    current_prinny->animation_id = animation;
    current_prinny->frame_index = rand() % animations[animation].count;
    current_prinny->speed = rand() % 60 + 120;
}

void push_n_random_prinnies(int n) {
    for (int i = 0; i < n; ++i) {
        push_random_prinny();
    }
}

void blackiron_monkey_host_request_render_limit_preferences(struct graphics_context_limits* limits) {
    limits->glyph_cache = 8192;
    limits->batch_quads = 8192;
}

static char* TEST_FRAGMENT_SOURCE =
    "#version 330\n"
    "in vec2 vertex_position;\n"
    "in vec2 vertex_texcoord;\n"
    "in vec4 vertex_color;\n"
    "uniform sampler2D sampler_texture;\n"
    "uniform float     using_texture = 0;"
    "out vec4 output_color;\n"
    "void main(){\n"
    // branchless shader to do texture sampling.
    "vec4 sample_color = mix(vertex_color, texture(sampler_texture, vertex_texcoord) * vertex_color, using_texture);\n"
    "vec4 grayscale = vec4((sample_color.r + sample_color.g + sample_color.b) / 3);"
    "grayscale.a = 1.0;"
    "output_color = grayscale * sample_color.a;"
    /* "output_color = vec4(1, 0, 0 , 1);" */
    "}\n"
    ;

void blackiron_monkey_host_initialize(struct host_state* state, int arugment_count, char** argument_values) {
    graphics = state->graphics;
    system   = state->system;
    input    = state->input;

    srand(time(0));

    arial_font         = graphics->load_font_from_file("resources/arial.ttf", 48);
    smaller_arial_font = graphics->load_font_from_file("resources/arial.ttf", 24);
    alkhemikal_font    = graphics->load_font_from_file("resources/Alkhemikal.ttf", 48);
    jp_font            = graphics->load_font_from_file("resources/NotoSerifJP-Regular.otf", 32);
    prinny_sheet       = graphics->load_texture_from_file("resources/testprinny/prinny_sheet.png");
    test_tile          = graphics->load_texture_from_file("resources/testtile.png");
    light_blob         = graphics->load_texture_from_file("resources/light_blob.png");


    /* grayscale_shader = graphics->load_shader_from_source(NULL, TEST_FRAGMENT_SOURCE); */

    graphics->set_virtual_resolution(1920, 1080);

    for (int i = 0; i < 16; ++i) {
        animations[i].atlas_texture = prinny_sheet;
    }

    prinnies = graphics->push_buffer_allocate_forward(1000000*sizeof(struct prinny_particle)).memory;
}

void blackiron_monkey_host_frame(struct host_state* state, float delta_time) {
    graphics->clear_buffer(0.0f, 0.0f, 0.f, 1.0f);

    // setup state, if there is any.
    static float camera_x = 0;
    static float camera_y = 0;
    static float camera_scale = 1.0;
    static bool camera_think = false;
    static bool prinny_update_think = true;

    bool allow_input = !state->console->active();

    if (allow_input) {
        if (input->key_pressed(INPUT_KEY_1)) {
            push_n_random_prinnies(100);
        } else if (input->key_pressed(INPUT_KEY_2)) {
            push_n_random_prinnies(500);
        } else if (input->key_pressed(INPUT_KEY_3)) {
            push_n_random_prinnies(1000);
        } else if (input->key_pressed(INPUT_KEY_4)) {
            push_n_random_prinnies(10000);
        } else if (input->key_pressed(INPUT_KEY_5)) {
            push_n_random_prinnies(100000);
        } else if (input->key_pressed(INPUT_KEY_K)) {
            prinny_count = 0;
            light_count = 0;
        } else if (input->key_pressed( INPUT_KEY_L)) {
            int random_color = rand() % 4;
            switch (random_color) {
                case 0: {light_push(camera_x, camera_y, 100.0, 1.0, 1.0, 1.0);} break;
                case 1: {light_push(camera_x, camera_y, 100.0, 1.0, 0.0, 0.0);} break;
                case 2: {light_push(camera_x, camera_y, 100.0, 0.0, 1.0, 1.0);} break;
                case 3: {light_push(camera_x, camera_y, 100.0, 1.0, 0.0, 1.0);} break;
            }
        }

        if (input->key_pressed(INPUT_KEY_E)) {
            camera_think ^= 1;
        }

        if (input->key_pressed(INPUT_KEY_R)) {
            prinny_update_think ^= 1;
        }
    }

    if (camera_think) {
        camera_x = sinf(state->elapsed_time/4) * 300;
        camera_y = cosf(state->elapsed_time/8) * 100 - 200;
        camera_scale = clamp_float((sinf(state->elapsed_time/4) + 1.6) / 2.0, 0.6, 1.5);
    } else {
        // maintain the same relative speed
        // base on scale.
        float speed = 500 / camera_scale;
        if (allow_input) {
            if (input->key_down(INPUT_KEY_W)) {
                camera_y -= speed * delta_time;
            }

            if (input->key_down(INPUT_KEY_S)) {
                camera_y += speed * delta_time;
            }

            if (input->key_down(INPUT_KEY_A)) {
                camera_x -= speed * delta_time;
            }

            if (input->key_down(INPUT_KEY_D)) {
                camera_x += speed * delta_time;
            }
        }

        {
            float new_scale = camera_scale;

            if (allow_input) {
                if (input->key_down(INPUT_KEY_UP)) {
                    new_scale = clamp_float(camera_scale + delta_time * 1.5, 0.2, 5);
                }

                if (input->key_down(INPUT_KEY_DOWN)) {
                    new_scale = clamp_float(camera_scale - delta_time * 1.5, 0.2, 5);
                }
            }

            camera_scale = new_scale;
        }
    }

    graphics->set_virtual_resolution(1280, 720);
    struct graphics_context_screen_dimensions dimensions = graphics->screen_dimensions();
    
    if (prinny_update_think) {
        prinny_particles_update(50 * 64, dimensions.height, delta_time, prinnies, prinny_count);
    }
    graphics->begin_drawing((struct camera) {.scale_x = 1, .scale_y = 1});
    graphics->draw_untextured_quad(0, 0, 9999, 9999, 0, 0, 0.1, 1, null_shader);
    graphics->end_drawing();

    struct graphics_context_virtual_dimensions virtual_dimensions = graphics->virtual_dimensions();
    graphics->begin_drawing((struct camera) {
            .x = (camera_x*camera_scale) - virtual_dimensions.width/2, .y = (camera_y*camera_scale) - virtual_dimensions.height/2,
            .scale_x = camera_scale, .scale_y = camera_scale,
        });

    for (int y = 0; y < 50; ++y) {
        for (int x = 0; x < 50; ++x) {
            struct color brightness = calculate_basic_lighting(x*64, y*32, lights, light_count);
            graphics->draw_textured_quad(test_tile, x*64, y*28, 64, 64, brightness.r, brightness.g, brightness.b, 1, null_shader);
        }
    }
    prinny_particles_render(prinnies, prinny_count);

    for (size_t light_index = 0; light_index < light_count; ++light_index) {
        float pulse = ((sinf(state->elapsed_time*7 + 30) + 1.0)/2.0);
        float pulse2 = ((sinf(state->elapsed_time*1) + 1.0)/2.0);
        float w = 256 + pulse2 * 100;
        float h = 256 + pulse2 * 100;
        graphics->draw_textured_quad(light_blob, lights[light_index].x-w/2, lights[light_index].y-h/2, w, h, lights[light_index].r, lights[light_index].g, lights[light_index].b, pulse * 0.4 + 0.5, null_shader);
    }

    graphics->end_drawing();

    graphics->begin_drawing((struct camera) {.scale_x = 1, .scale_y = 1,});
    {
        float y = 0; 
        graphics->draw_text(arial_font, immediate_format_text("FPS: %d", (int32_t)system->average_framerate()), 0, y, 1.0, 0.0, 1.0, 0.0, 1.0);
        y += 32;
        graphics->draw_text(arial_font, immediate_format_text("prinny count: %d", prinny_count), 0, y, 1.0, 0.0, 1.0, 0.0, 1.0);
        y += 32;
        graphics->draw_text(arial_font, "PRINNY MARK DOOD!", 0, y, 1.0, 1.0, sinf(state->elapsed_time), 1.0, 1.0);
        y += 32;
        {
            size_t total = system->memory_allocated_total();
            graphics->draw_text(smaller_arial_font, immediate_format_text("memory allocated: %d bytes (%d KiB) (%d MiB) (%d GiB)", total, total / 1024, total / (1024*1024), total / (1024*1024*1024)), 0, y, 1.0, 0.0, 1.0, 0.0, 1.0);
            y += 16;
            char utf8_text[] = "私はガラスを食べられます。それは私を傷つけません。";
            graphics->draw_text(jp_font, utf8_text, 0, y, 1.0, 0.0, 1.0, 0.0, 1.0);
            y += 32;
        }

    }
    graphics->end_drawing();
    state->elapsed_time += delta_time;
}

void blackiron_monkey_host_deinitialize(struct host_state* state) {
}

struct host_application_description get_application_description(void) {
    return (struct host_application_description) {
        .title  = "Prinny Mark!",
        .window = {
            .width  = 1280,
            .height = 720,
            .fullscreen = false,
        },

        .functions = (struct host_api) {
            .initialize = blackiron_monkey_host_initialize,
            .query_render_preferences = blackiron_monkey_host_request_render_limit_preferences,
            .frame = blackiron_monkey_host_frame,
            .deinitialize = blackiron_monkey_host_deinitialize,
        }
    };
}
