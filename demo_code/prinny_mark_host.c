#include "public/common.h"
#include "public/configuration.h"

#include "public/graphics_context_api.h"
#include "public/renderer_api.h"
#include "public/system_api.h"
#include "public/input_api.h"

#include "public/host_state.h"

static struct renderer_api* renderer_api;
static struct renderer*     renderer;

static struct graphics_context_api* graphics;
static struct input_api*    input;
static struct system_api*   system;

graphics_context_font_handle    arial_font;
graphics_context_font_handle    jp_font;
graphics_context_font_handle    smaller_arial_font;

graphics_context_texture_handle prinny_sheet;

graphics_context_texture_handle light_blob;
graphics_context_texture_handle test_tile;

enum prinny_animation_id {
    PRINNY_ANIMATION_ID_IDLE,
    PRINNY_ANIMATION_ID_SPINNING,
    PRINNY_ANIMATION_ID_YELP,
    PRINNY_ANIMATION_ID_COUNT,
};
// montage image
// 512 / 4 = 128
// 504 / 4 = 126

static struct renderer_sprite_subsprite idle_rects[1] = {
    (struct renderer_sprite_subsprite) {0, 0, 128, 126}
};
static struct renderer_sprite_animation_frame idle_frames[1];
static struct renderer_sprite_subsprite yelp_rects[1] = {
    (struct renderer_sprite_subsprite) {3*128, 3*126, 128, 126}
};
static struct renderer_sprite_animation_frame yelp_frames[1];
//obviously in real games please have tools generate this.
// this is a pain to do by hand lol.
static struct renderer_sprite_subsprite spinning_rects[14] = {
    (struct renderer_sprite_subsprite) {1*128,     0, 128, 126},
    (struct renderer_sprite_subsprite) {2*128,     0, 128, 126},
    (struct renderer_sprite_subsprite) {0,     1*126, 128, 126},
    (struct renderer_sprite_subsprite) {2*128, 1*126, 128, 126},
    (struct renderer_sprite_subsprite) {0,     2*126, 128, 126},
    (struct renderer_sprite_subsprite) {2*128, 2*126, 128, 126},
    (struct renderer_sprite_subsprite) {0,     3*126, 128, 126},
    (struct renderer_sprite_subsprite) {2*128, 3*126, 128, 126},
    (struct renderer_sprite_subsprite) {1*128, 3*126, 128, 126},
    (struct renderer_sprite_subsprite) {3*128, 2*126, 128, 126},
    (struct renderer_sprite_subsprite) {1*128, 2*126, 128, 126},
    (struct renderer_sprite_subsprite) {3*128, 1*126, 128, 126},
    (struct renderer_sprite_subsprite) {1*128, 1*126, 128, 126},
    (struct renderer_sprite_subsprite) {3*128,     0, 128, 126},
};
static struct renderer_sprite_animation_frame spinning_frames[14];

struct renderer_sprite_animation animations[8];
struct prinny_particle {
    uint8_t animation_id;
    renderer_sprite_handle sprite;
    int16_t speed;

    bool going_forward;

    float x;
    float y;
};

static void prinny_particles_update(int screen_width, int screen_height, float delta_time, struct prinny_particle* prinnies, size_t count) {
    for (size_t prinny_index = 0; prinny_index < count; ++prinny_index) {
        struct prinny_particle* current_prinny = &prinnies[prinny_index];

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

        {
            struct renderer_sprite* sprite_object = renderer_api->dereference_sprite(renderer, current_prinny->sprite);
            sprite_object->playing = true;
            sprite_object->x = current_prinny->x;
            sprite_object->y = current_prinny->y;
        }
    }
}

size_t prinny_count = 0;
struct prinny_particle* prinnies;

void push_random_prinny(void) {
    struct prinny_particle* current_prinny = &prinnies[prinny_count++];
    current_prinny->x = rand() % (64 * 50);
    current_prinny->y = rand() % (30 * 50);

    uint16_t animation = rand() % PRINNY_ANIMATION_ID_COUNT;
    current_prinny->animation_id = animation;
    {
        renderer_sprite_handle sprite_handle = renderer_api->new_sprite(renderer);
        struct renderer_sprite* sprite_object = renderer_api->dereference_sprite(renderer, sprite_handle);
        sprite_object->frame_index = rand() % animations[animation].frame_count;
        sprite_object->centered = true;
        sprite_object->w = 128/4;
        sprite_object->h = 126/4;
        sprite_object->rgba = 0xFFFFFFFF;
        current_prinny->sprite = sprite_handle;
        renderer_api->attach_self_managed_animation_to_sprite(sprite_object, &animations[animation]);
    }
    current_prinny->speed = rand() % 60 + 50;
}

void push_n_random_prinnies(int n) {
    for (int i = 0; i < n; ++i) {
        push_random_prinny();
    }
}

void blackiron_monkey_host_request_render_limit_preferences(struct graphics_context_limits* limits) {
    limits->glyph_cache     = 4096;
    limits->batch_quads     = 8192;
    limits->memory_capacity = Mebibyte(128);
}

static void rebuild_render_world(void) {
    for (int y = 0; y < 30; ++y) {
        for (int x = 0; x < 30; ++x) {
            renderer_sprite_handle tile = renderer_api->new_sprite_with_texture(renderer, test_tile);
            struct renderer_sprite* sprite_resource = renderer_api->dereference_sprite(renderer, tile);
            sprite_resource->centered = true;
            sprite_resource->x = x * 64;
            sprite_resource->y = y * 32;
            sprite_resource->w = 64;
            sprite_resource->h = 64;
            if (x == 0) {
                sprite_resource->rgba = 0xFFFFFFFF;
            } else if (x == 1) {
                sprite_resource->rgba = 0xFFFF00FF;
            } else {
                sprite_resource->rgba = 0xFF00FFFF;
            }
        }
    }
}
void blackiron_monkey_host_initialize(struct host_state* state, int arugment_count, char** argument_values) {
    graphics     = state->graphics;
    renderer_api = state->renderer_api;
    input        = state->input;
    system       = state->system;

    arial_font         = graphics->load_font_from_file("resources/arial.ttf", 32);
    smaller_arial_font = graphics->load_font_from_file("resources/arial.ttf", 16);
    jp_font            = graphics->load_font_from_file("resources/NotoSerifJP-Regular.otf", 32);

    #define count 1000000
    renderer = renderer_api->create((struct renderer_limits) {.lights = 64, .sprites = count+1500});
    prinnies = system->memory_allocate(count * sizeof(struct prinny_particle));
    #undef count
    
    srand(time(0));
    prinny_sheet = graphics->load_texture_from_file("resources/testprinny/prinny_sheet.png");
    test_tile    = graphics->load_texture_from_file("resources/testtile.png");
    light_blob   = graphics->load_texture_from_file("resources/light_blob.png");


    {
        idle_frames[0].texture = prinny_sheet;
        idle_frames[0].subsprite = idle_rects[0];
    }
    {
        yelp_frames[0].texture = prinny_sheet;
        yelp_frames[0].subsprite = yelp_rects[0];
    }
    {
        for (unsigned index = 0; index < 14; ++index) {
            spinning_frames[index].texture   = prinny_sheet;
            spinning_frames[index].subsprite = spinning_rects[index];
            spinning_frames[index].time_until_next_frame = 0.075;
        }
    }

    {
        animations[PRINNY_ANIMATION_ID_IDLE].frame_count = 1;
        animations[PRINNY_ANIMATION_ID_IDLE].frames      = idle_frames;
    }
    {
        animations[PRINNY_ANIMATION_ID_YELP].frame_count = 1;
        animations[PRINNY_ANIMATION_ID_YELP].frames      = yelp_frames;
    }
    {
        animations[PRINNY_ANIMATION_ID_SPINNING].frame_count = 14;
        animations[PRINNY_ANIMATION_ID_SPINNING].frames      = spinning_frames;
    }
    rebuild_render_world();
}

void blackiron_monkey_host_frame(struct host_state* state, float delta_time) {
    static float camera_x = 200;
    static float camera_y = 200;
    static float camera_scale = 1.0;
    static bool camera_think = false;
    static bool prinny_update_think = true;

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
        renderer_api->clear_lights(renderer);
        for (size_t i = 0; i < prinny_count; ++i) {
            renderer_api->free_sprite(renderer, prinnies[i].sprite);
        } prinny_count = 0;
    } else if (input->key_pressed(INPUT_KEY_L)) {
        int random_color = rand() % 4;
        struct renderer_light* new_light = renderer_api->dereference_light(renderer, renderer_api->new_light(renderer));
        new_light->x = camera_x; new_light->y = camera_y;
        switch (random_color) {
            case 0: {new_light->rgba = 0xFFFFFFFF; new_light->power = 100;} break;
            case 1: {new_light->rgba = 0xFF0000FF; new_light->power = 120;} break;
            case 2: {new_light->rgba = 0x00FFFFFF; new_light->power = 100;} break;
            case 3: {new_light->rgba = 0x0000FFFF; new_light->power = 100;} break;
        }
    }

    if (input->key_pressed(INPUT_KEY_E)) {
        camera_think ^= 1;
    }

    if (input->key_pressed(INPUT_KEY_R)) {
        prinny_update_think ^= 1;
    }

    if (camera_think) {
        camera_x = sinf(state->elapsed_time/4) * 300;
        camera_y = cosf(state->elapsed_time/8) * 100 - 200;
        camera_scale = clamp_float((sinf(state->elapsed_time/4) + 1.6) / 2.0, 0.6, 1.5);
    } else {
        // maintain the same relative speed
        // base on scale.
        float speed = 500 / camera_scale;
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

        {
            float new_scale = camera_scale;
            if (input->key_down(INPUT_KEY_UP)) {
                new_scale = clamp_float(camera_scale + delta_time * 1.5, 0.2, 5);
            }

            if (input->key_down(INPUT_KEY_DOWN)) {
                new_scale = clamp_float(camera_scale - delta_time * 1.5, 0.2, 5);
            }

            camera_scale = new_scale;
        }
    }

    struct graphics_context_virtual_dimensions virtual_dimensions = graphics->virtual_dimensions();
    if (prinny_update_think) {
        prinny_particles_update(50 * 64, virtual_dimensions.height, delta_time, prinnies, prinny_count);
    } else {
        for (size_t i = 0; i < prinny_count; ++i) {
            struct prinny_particle* current_prinny = &prinnies[i];
            {
                struct renderer_sprite* sprite_object = renderer_api->dereference_sprite(renderer, current_prinny->sprite);
                sprite_object->playing = false;
            }
        }
    }

    #if 1
    renderer_api->set_clear_color(renderer, 0, 0, 0);
    renderer_api->set_camera_position(renderer, camera_x, camera_y);
    renderer_api->set_camera_scale(renderer, camera_scale);
    renderer_api->update(renderer, delta_time);
    renderer_api->flush(renderer);
    #endif

    #if 1
    graphics->begin_drawing((struct camera) {.scale_x = 1, .scale_y = 1,});
    /* printf("%s\n", immediate_format_text("FPS: %d", (int32_t)system->average_framerate())); */
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
            graphics->draw_codepoint(jp_font, 12016, 0, y+32, 1.0, 0.0, 1.0, 0.0, 1.0);
        }

    }
    graphics->end_drawing();
    #endif
    state->elapsed_time += delta_time;
}

void blackiron_monkey_host_deinitialize(struct host_state* state) {
}

struct host_api client_get_host_api(void) {
    return (struct host_api) {
        .initialize               = blackiron_monkey_host_initialize,
        .query_render_preferences = blackiron_monkey_host_request_render_limit_preferences,
        .frame                    = blackiron_monkey_host_frame,
        .deinitialize             = blackiron_monkey_host_deinitialize,
    };
}
