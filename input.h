#ifndef INPUT_H
#define INPUT_H

#include "public/input_api.h"

// These can be more compact, but I don't really care about that right now
struct keyboard_input_state {
    // TODO(jerry):
    // right now I'm ignoring modifiers
    // this could be important in the future so make note of that.
    bool down[INPUT_KEY_COUNT];
};

struct mouse_input_state {
    bool buttons[MOUSE_BUTTON_COUNT];

    int32_t x;
    int32_t y;
};

struct controller_input_state  {
    uint8_t id;

    struct controller_thumbstick_state left_thumbstick;
    struct controller_thumbstick_state right_thumbstick;

    int8_t left_trigger_pressure;
    int8_t right_trigger_pressure;

    bool buttons[INPUT_CONTROLLER_BUTTON_COUNT];
};

struct input_state {
    struct keyboard_input_state keys;
    struct mouse_input_state    mouse;

    uint8_t controller_count;
    struct controller_input_state controllers[4];
};

struct input {
    struct input_state last;
    struct input_state current;
};

bool input_controller_valid_controller(struct input* input, uint8_t controller_index);

bool input_is_controller_button_down(struct input* input, uint8_t controller_index, int32_t button);
bool input_is_controller_button_pressed(struct input* input, uint8_t controller_index, int32_t button);
struct controller_thumbstick_state input_controller_left_thumbstick(struct input* input, uint8_t controller_index);
struct controller_thumbstick_state input_controller_right_thumbstick(struct input* input, uint8_t controller_index);
uint8_t input_controller_left_trigger_pressure(struct input* input, uint8_t controller_index);
uint8_t input_controller_right_trigger_pressure(struct input* input, uint8_t controller_index);

bool input_is_key_down(struct input* input, int32_t key);
bool input_is_key_pressed(struct input* input, int32_t key);

bool input_is_mouse_button_down(struct input* input, int32_t button);
int32_t input_mouse_x(struct input* input);
int32_t input_mouse_y(struct input* input);

void    input_reset_all_input_state(struct input* input);

// platform specific functionality to implement
// these shouldn't be exported symbols, as they should only be used in the platform
// layer once.
static void input_poll_for_new_controllers(struct input* input);
static void input_update_controller_states(struct input* input);
// There is no cooresponding one for the controllers because there are far fewer controller
// buttons to remap so they are just inlined.
static int32_t input_map_native_keycode_to_input_keycode(int32_t native_keycode);

#endif
