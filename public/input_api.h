#ifndef INPUT_API_H
#define INPUT_API_H

enum input_key {
    INPUT_KEY_UNKNOWN,

    INPUT_KEY_A, INPUT_KEY_B, INPUT_KEY_C, INPUT_KEY_D, INPUT_KEY_E, INPUT_KEY_F, INPUT_KEY_G,
    INPUT_KEY_H, INPUT_KEY_I, INPUT_KEY_J, INPUT_KEY_K, INPUT_KEY_L, INPUT_KEY_M, INPUT_KEY_N,
    INPUT_KEY_O, INPUT_KEY_P, INPUT_KEY_Q, INPUT_KEY_R, INPUT_KEY_S, INPUT_KEY_T, INPUT_KEY_U,
    INPUT_KEY_V, INPUT_KEY_W, INPUT_KEY_X, INPUT_KEY_Y, INPUT_KEY_Z,

    INPUT_KEY_F1, INPUT_KEY_F2, INPUT_KEY_F3, INPUT_KEY_F4, INPUT_KEY_F5,
    INPUT_KEY_F6, INPUT_KEY_F7, INPUT_KEY_F8, INPUT_KEY_F9, INPUT_KEY_F10,
    INPUT_KEY_F11, INPUT_KEY_F12,

    INPUT_KEY_UP, INPUT_KEY_DOWN, INPUT_KEY_RIGHT, INPUT_KEY_LEFT,

    INPUT_KEY_0, INPUT_KEY_1, INPUT_KEY_2, INPUT_KEY_3, INPUT_KEY_4, INPUT_KEY_5,
    INPUT_KEY_6, INPUT_KEY_7, INPUT_KEY_8, INPUT_KEY_9,

    INPUT_KEY_MINUS, INPUT_KEY_BACKQUOTE, INPUT_KEY_EQUALS,
    INPUT_KEY_SEMICOLON, INPUT_KEY_QUOTE, INPUT_KEY_COMMA,
    INPUT_KEY_PERIOD,

    INPUT_KEY_RETURN, INPUT_KEY_BACKSPACE, INPUT_KEY_ESCAPE,

    INPUT_KEY_INSERT, INPUT_KEY_HOME, INPUT_KEY_PAGEUP, INPUT_KEY_PAGEDOWN, INPUT_KEY_DELETE, INPUT_KEY_END,
    INPUT_KEY_PRINTSCREEN,

    INPUT_KEY_PAUSE,
    INPUT_KEY_SCROLL_LOCK,
    INPUT_KEY_NUMBER_LOCK,

    INPUT_KEYPAD_0, INPUT_KEYPAD_1, INPUT_KEYPAD_2, INPUT_KEYPAD_3, INPUT_KEYPAD_4,
    INPUT_KEYPAD_5, INPUT_KEYPAD_6, INPUT_KEYPAD_7, INPUT_KEYPAD_8, INPUT_KEYPAD_9,

    INPUT_KEYPAD_LEFT, INPUT_KEYPAD_RIGHT, INPUT_KEYPAD_UP, INPUT_KEYPAD_DOWN,

    INPUT_KEYPAD_ASTERISK, INPUT_KEYPAD_BACKSLASH,
    INPUT_KEYPAD_MINUS, INPUT_KEYPAD_PLUS, INPUT_KEYPAD_PERIOD,

    INPUT_KEY_LEFT_BRACKET, INPUT_KEY_RIGHT_BRACKET,
    INPUT_KEY_FORWARDSLASH, INPUT_KEY_BACKSLASH,

    INPUT_KEY_TAB, 
    INPUT_KEY_SHIFT,

    INPUT_KEY_META, INPUT_KEY_SUPER, INPUT_KEY_SPACE,

    INPUT_KEY_CTRL, INPUT_KEY_ALT,

    // do not use this
    INPUT_KEY_LCTRL, INPUT_KEY_RCTRL,
    INPUT_KEY_LALT, INPUT_KEY_RALT,
    INPUT_KEY_LSHIFT, INPUT_KEY_RSHIFT,

    INPUT_KEY_COUNT
};

// KEYPAD keys are left out because I have not mapped them yet.
static char* input_key_strings[] = {
    [INPUT_KEY_UNKNOWN] = "Unknown Key?",

    [INPUT_KEY_A] = "A",
    [INPUT_KEY_B] = "B",
    [INPUT_KEY_C] = "C",
    [INPUT_KEY_D] = "D",
    [INPUT_KEY_E] = "E",
    [INPUT_KEY_F] = "F",
    [INPUT_KEY_G] = "G",
    [INPUT_KEY_H] = "H",
    [INPUT_KEY_I] = "I",
    [INPUT_KEY_J] = "J",
    [INPUT_KEY_K] = "K",
    [INPUT_KEY_L] = "L",
    [INPUT_KEY_M] = "M",
    [INPUT_KEY_N] = "N",
    [INPUT_KEY_O] = "O",
    [INPUT_KEY_P] = "P",
    [INPUT_KEY_Q] = "Q",
    [INPUT_KEY_R] = "R",
    [INPUT_KEY_S] = "S",
    [INPUT_KEY_T] = "T",
    [INPUT_KEY_U] = "U",
    [INPUT_KEY_V] = "V",
    [INPUT_KEY_W] = "W",
    [INPUT_KEY_X] = "X",
    [INPUT_KEY_Y] = "Y",
    [INPUT_KEY_Z] = "Z",

    [INPUT_KEY_F1]  = "F1",
    [INPUT_KEY_F2]  = "F2",
    [INPUT_KEY_F3]  = "F3",
    [INPUT_KEY_F4]  = "F4",
    [INPUT_KEY_F5]  = "F5",
    [INPUT_KEY_F6]  = "F6",
    [INPUT_KEY_F7]  = "F7",
    [INPUT_KEY_F8]  = "F8",
    [INPUT_KEY_F9]  = "F9",
    [INPUT_KEY_F10] = "F10",
    [INPUT_KEY_F11] = "F11",
    [INPUT_KEY_F12] = "F12",

    [INPUT_KEY_MINUS]     = "-",
    [INPUT_KEY_BACKQUOTE] = "`",
    [INPUT_KEY_EQUALS]    = "=",
    [INPUT_KEY_SEMICOLON] = ";",
    [INPUT_KEY_QUOTE]     = "\'",
    [INPUT_KEY_COMMA]     = ",",
    [INPUT_KEY_PERIOD]    = ".",

    [INPUT_KEY_RETURN]    = "Return",
    [INPUT_KEY_BACKSPACE] = "Backspace",
    [INPUT_KEY_ESCAPE]    = "Escape",

    [INPUT_KEY_INSERT]   = "Insert",
    [INPUT_KEY_HOME]     = "Home",
    [INPUT_KEY_PAGEUP]   = "Page Up",
    [INPUT_KEY_PAGEDOWN] = "Page Down",
    [INPUT_KEY_DELETE]   = "Delete",
    [INPUT_KEY_END]      = "End",

    [INPUT_KEY_PRINTSCREEN] = "Print Screen",
    [INPUT_KEY_PAUSE]       = "Pause",
    
    [INPUT_KEY_SCROLL_LOCK] = "Scroll Lock",
    [INPUT_KEY_NUMBER_LOCK] = "Number Lock",

    [INPUT_KEY_LEFT_BRACKET]  = "[",
    [INPUT_KEY_RIGHT_BRACKET] = "]",

    [INPUT_KEY_FORWARDSLASH] = "/",
    [INPUT_KEY_BACKSLASH]    = "\\",

    [INPUT_KEY_TAB]   = "Tab",
    [INPUT_KEY_SHIFT] = "Shift",
    [INPUT_KEY_CTRL]  = "Control",
    [INPUT_KEY_ALT]   = "Alt",

    [INPUT_KEY_SPACE] = "Space",
};

enum mouse_buttons {
    MOUSE_BUTTON_LEFT,
    MOUSE_BUTTON_MIDDLE,
    MOUSE_BUTTON_RIGHT,
    MOUSE_BUTTON_COUNT,
};
static char* mouse_button_strings[] = {
    "Mouse Left",
    "Mouse Middle",
    "Mouse Right",
};

// Although I love playstation, standard nomenclature is to probably use Xbox terminology.
enum input_controller_button {
    INPUT_CONTROLLER_A,
    INPUT_CONTROLLER_B,
    INPUT_CONTROLLER_X,
    INPUT_CONTROLLER_Y,

    // For some reason these are also called "hats"
    INPUT_CONTROLLER_DPAD_LEFT,
    INPUT_CONTROLLER_DPAD_UP,
    INPUT_CONTROLLER_DPAD_RIGHT,
    INPUT_CONTROLLER_DPAD_DOWN,

    // (L3, R3) clicking on the thumbsticks.
    INPUT_CONTROLLER_RIGHT_THUMB,
    INPUT_CONTROLLER_LEFT_THUMB,

    // These are the shoulder buttons. (RT, LT / R2, L2)
    INPUT_CONTROLLER_RIGHT_TRIGGER,
    INPUT_CONTROLLER_LEFT_TRIGGER,

    INPUT_CONTROLLER_START,
    // Select / Share button on Playstation
    INPUT_CONTROLLER_BACK,

    INPUT_CONTROLLER_BUTTON_COUNT
};

static char* input_controller_button_strings[] = {
    "Controller A",
    "Controller B",
    "Controller X",
    "Controller Y",

    "Controller D-Pad Left",
    "Controller D-Pad Up",
    "Controller D-Pad Right",
    "Controller D-Pad Down",

    "Controller Right Thumb",
    "Controller Left Thumb",

    "Controller Right Trigger",
    "Controller Left Trigger",

    "Controller Start",
    "Controller Back",
};

// positive right axis, negative left axis
// positive up axis, negative down axis
#define THUMBSTICK_MAX_MAGNITUDE (32768.0)
struct controller_thumbstick_state {
    int16_t x;
    int16_t y;
};

struct input_api {
    bool (*valid_controller)(uint8_t);
    bool (*controller_button_down)(uint8_t, int32_t);
    bool (*controller_button_pressed)(uint8_t, int32_t);

    struct controller_thumbstick_state (*controller_left_thumbstick)(uint8_t);
    struct controller_thumbstick_state (*controller_right_thumbstick)(uint8_t);

    uint8_t (*controller_left_trigger_pressure)(uint8_t);
    uint8_t (*controller_right_trigger_pressure)(uint8_t);

    bool (*key_down)(int32_t);
    bool (*key_pressed)(int32_t);

    int32_t (*mouse_x)(void);
    int32_t (*mouse_y)(void);
};

#endif
