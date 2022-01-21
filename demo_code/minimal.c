// template code!
#include "public/common.h"

#include "public/configuration.h"

#include "public/graphics_context_api.h"
#include "public/system_api.h"
#include "public/input_api.h"

#include "public/host_state.h"

#include "public/vector2d.h"
#include "public/collision.h"

static struct console_api*          console;
static struct graphics_context_api* graphics;
static struct input_api*            input;
static struct system_api*           system;

void request_render_limit_preferences(struct graphics_context_limits* limits) {
}

void initialize(struct host_state* state, int arugment_count, char** argument_values) {
    graphics     = state->graphics;
    input        = state->input;
    system       = state->system;
    console      = state->console;
}

void frame(struct host_state* state, float delta_time) {
    graphics->clear_buffer(0.0, 0.0, 0.3, 1.0);
    state->elapsed_time += delta_time;
}

void deinitialize(struct host_state* state) {
}

struct host_application_description get_application_description(void) {
    return (struct host_application_description) {
        .title  = "blah blah",
        .window = {
            .width  = 1280,
            .height = 720,
            /* .fullscreen = true, */
        },

        .functions = (struct host_api) {
            .initialize               = initialize,
            .query_render_preferences = request_render_limit_preferences,
            .frame                    = frame,
            .deinitialize             = deinitialize,
        }
    };
}
