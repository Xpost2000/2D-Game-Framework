bool input_is_mouse_button_down(struct input* input, int32_t button) {
    return input->current.mouse.buttons[button];
}

int32_t input_mouse_x(struct input* input) {
    return input->current.mouse.x;
}

int32_t input_mouse_y(struct input* input) {
    return input->current.mouse.y;
}

bool input_is_key_down(struct input* input, int32_t key) {
    return input->current.keys.down[key];
}

bool input_is_key_pressed(struct input* input, int32_t key) {
    bool current_down = input->current.keys.down[key]; 
    bool last_down    = input->last.keys.down[key]; 

    return (!current_down && last_down);
}

bool input_controller_valid_controller(struct input* input, uint8_t controller_index) {
    return (input->current.controller_count > controller_index);
} 

struct controller_thumbstick_state input_controller_left_thumbstick(struct input* input, uint8_t controller_index) {
    return input->current.controllers[controller_index].left_thumbstick;
}

struct controller_thumbstick_state input_controller_right_thumbstick(struct input* input, uint8_t controller_index) {
    return input->current.controllers[controller_index].right_thumbstick;
}

uint8_t input_controller_left_trigger_pressure(struct input* input, uint8_t controller_index) {
    return input->current.controllers[controller_index].left_trigger_pressure;
}

uint8_t input_controller_right_trigger_pressure(struct input* input, uint8_t controller_index) {
    return input->current.controllers[controller_index].right_trigger_pressure;
}

bool input_is_controller_button_down(struct input* input, uint8_t controller_index, int32_t button) {
    return input->current.controllers[controller_index].buttons[button];
}

bool input_is_controller_button_pressed(struct input* input, uint8_t controller_index, int32_t button) {
    // I should check if they were actually the same controller.
    // IE: I need to find the cooresponding controller lol.
    bool current_down = input->current.controllers[controller_index].buttons[button];
    bool last_down    = input->last.controllers[controller_index].buttons[button];

    return (!current_down && last_down);
}

void input_reset_all_input_state(struct input* input) {
    memset(input->current.keys.down, 0, sizeof(input->current.keys));
}
