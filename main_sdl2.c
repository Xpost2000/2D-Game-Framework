// Hi, I'm the SDL platform layer.
// 

// This is used for any platform I don't know
// some specifics about, and is probably the platform for emscripten!

// This will also work on windows but obviously this platform target requires
// SDL2 as an external dependency!

// on windows I will probably have both SDL and non SDL executables. They would basically
// be identical. It's just choice I suppose.
#if __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif
#include <SDL2/SDL.h>
// Why does SDL also have this issue?
// grrr.
#undef near
#undef far

// As this is a unity build, the order this is setup. Is pretty important.
// This is really the only finicky thing, but it doesn't take very long to figure out
// the order of inclusion anyways.
int blackiron_main(int, char**);

static void* platform_load_shared_object(char* file_location) {
    // Hmm... Why does SDL2 not filter an empty string out?
    if (strlen(file_location) == 0) {
        return NULL;
    }

    void* handle = SDL_LoadObject(file_location);
    return handle;
}
static void* platform_load_function_from_shared_object(void* shared_object, char* function_name) {
    return SDL_LoadFunction(shared_object, function_name);
}
static void platform_unload_shared_object(void* handle) {
    SDL_UnloadObject(handle);
}

SDL_Window* global_window_handle;

#include "platform_prelude.c"

void platform_set_window_title(char* title) {
    SDL_SetWindowTitle(global_window_handle, title);
}

void platform_set_window_resolution(int32_t width, int32_t height) {
    SDL_SetWindowSize(global_window_handle, width, height);
}

void platform_set_window_fullscreen(bool value) {
    if (value) {
        SDL_SetWindowFullscreen(global_window_handle, SDL_WINDOW_FULLSCREEN_DESKTOP);
    } else {
        SDL_SetWindowFullscreen(global_window_handle, 0);
    }
}

void platform_set_window_resizable(bool value) {
    SDL_SetWindowResizable(global_window_handle, value);
}

int main(int argc, char** argv) {
    int result = blackiron_main(argc, argv);
    return result;
}
static uint8_t _sdl_scancode_to_input_keycode_table[255] = {
    // top row
    [SDL_SCANCODE_ESCAPE]    = INPUT_KEY_ESCAPE,
    [SDL_SCANCODE_1]         = INPUT_KEY_1,
    [SDL_SCANCODE_2]         = INPUT_KEY_2,
    [SDL_SCANCODE_3]         = INPUT_KEY_3,
    [SDL_SCANCODE_4]         = INPUT_KEY_4,
    [SDL_SCANCODE_5]         = INPUT_KEY_5,
    [SDL_SCANCODE_6]         = INPUT_KEY_6,
    [SDL_SCANCODE_7]         = INPUT_KEY_7,
    [SDL_SCANCODE_8]         = INPUT_KEY_8,
    [SDL_SCANCODE_9]         = INPUT_KEY_9,
    [SDL_SCANCODE_0]         = INPUT_KEY_0,
    [SDL_SCANCODE_MINUS]     = INPUT_KEY_MINUS,
    [SDL_SCANCODE_EQUALS]    = INPUT_KEY_EQUALS,
    [SDL_SCANCODE_BACKSPACE] = INPUT_KEY_BACKSPACE,

    [SDL_SCANCODE_TAB] = INPUT_KEY_TAB,

    [SDL_SCANCODE_Q]            = INPUT_KEY_Q,
    [SDL_SCANCODE_W]            = INPUT_KEY_W,
    [SDL_SCANCODE_E]            = INPUT_KEY_E,
    [SDL_SCANCODE_R]            = INPUT_KEY_R,
    [SDL_SCANCODE_T]            = INPUT_KEY_T,
    [SDL_SCANCODE_Y]            = INPUT_KEY_Y,
    [SDL_SCANCODE_U]            = INPUT_KEY_U,
    [SDL_SCANCODE_I]            = INPUT_KEY_I,
    [SDL_SCANCODE_O]            = INPUT_KEY_O,
    [SDL_SCANCODE_P]            = INPUT_KEY_P,
    [SDL_SCANCODE_LEFTBRACKET]  = INPUT_KEY_LEFT_BRACKET,
    [SDL_SCANCODE_RIGHTBRACKET] = INPUT_KEY_RIGHT_BRACKET,
    [SDL_SCANCODE_RETURN]       = INPUT_KEY_RETURN,

    [SDL_SCANCODE_LCTRL] = INPUT_KEY_CTRL,
    [SDL_SCANCODE_RCTRL] = INPUT_KEY_CTRL,

    [SDL_SCANCODE_A] = INPUT_KEY_A,
    [SDL_SCANCODE_S] = INPUT_KEY_S,
    [SDL_SCANCODE_D] = INPUT_KEY_D,
    [SDL_SCANCODE_F] = INPUT_KEY_F,
    [SDL_SCANCODE_G] = INPUT_KEY_G,
    [SDL_SCANCODE_H] = INPUT_KEY_H,
    [SDL_SCANCODE_J] = INPUT_KEY_J,
    [SDL_SCANCODE_K] = INPUT_KEY_K,
    [SDL_SCANCODE_L] = INPUT_KEY_L,
    [SDL_SCANCODE_SEMICOLON] = INPUT_KEY_SEMICOLON,
    [SDL_SCANCODE_APOSTROPHE] = INPUT_KEY_QUOTE,
    [SDL_SCANCODE_GRAVE] = INPUT_KEY_BACKQUOTE,

    [SDL_SCANCODE_LSHIFT] = INPUT_KEY_SHIFT, // left shift
    [SDL_SCANCODE_RSHIFT] = INPUT_KEY_SHIFT, // left shift
    [SDL_SCANCODE_BACKSLASH] = INPUT_KEY_BACKSLASH,

    [SDL_SCANCODE_Z]      = INPUT_KEY_Z,
    [SDL_SCANCODE_X]      = INPUT_KEY_X,
    [SDL_SCANCODE_C]      = INPUT_KEY_C,
    [SDL_SCANCODE_V]      = INPUT_KEY_V,
    [SDL_SCANCODE_B]      = INPUT_KEY_B,
    [SDL_SCANCODE_N]      = INPUT_KEY_N,
    [SDL_SCANCODE_M]      = INPUT_KEY_M,
    [SDL_SCANCODE_COMMA]  = INPUT_KEY_COMMA,
    [SDL_SCANCODE_PERIOD] = INPUT_KEY_PERIOD,
    [SDL_SCANCODE_SLASH]  = INPUT_KEY_FORWARDSLASH,

    [SDL_SCANCODE_PRINTSCREEN] = INPUT_KEY_PRINTSCREEN,
    [SDL_SCANCODE_LALT]        = INPUT_KEY_ALT,
    [SDL_SCANCODE_RALT]        = INPUT_KEY_ALT,
    [SDL_SCANCODE_SPACE]       = INPUT_KEY_SPACE,

    [SDL_SCANCODE_F1]  = INPUT_KEY_F1,
    [SDL_SCANCODE_F2]  = INPUT_KEY_F2,
    [SDL_SCANCODE_F3]  = INPUT_KEY_F3,
    [SDL_SCANCODE_F4]  = INPUT_KEY_F4,
    [SDL_SCANCODE_F5]  = INPUT_KEY_F5,
    [SDL_SCANCODE_F6]  = INPUT_KEY_F6,
    [SDL_SCANCODE_F7]  = INPUT_KEY_F7,
    [SDL_SCANCODE_F8]  = INPUT_KEY_F8,
    [SDL_SCANCODE_F9]  = INPUT_KEY_F9,
    [SDL_SCANCODE_F10] = INPUT_KEY_F10,
    [SDL_SCANCODE_F11] = INPUT_KEY_F11,
    [SDL_SCANCODE_F12] = INPUT_KEY_F12,

    [SDL_SCANCODE_NUMLOCKCLEAR] = INPUT_KEY_NUMBER_LOCK,
    [SDL_SCANCODE_SCROLLLOCK]   = INPUT_KEY_SCROLL_LOCK,


    [SDL_SCANCODE_PAGEUP]   = INPUT_KEY_PAGEUP,
    [SDL_SCANCODE_HOME]     = INPUT_KEY_HOME,
    [SDL_SCANCODE_PAGEDOWN] = INPUT_KEY_PAGEDOWN,
    [SDL_SCANCODE_INSERT]   = INPUT_KEY_INSERT,
    [SDL_SCANCODE_DELETE]   = INPUT_KEY_DELETE,
    [SDL_SCANCODE_END]      = INPUT_KEY_END,

    [SDL_SCANCODE_UP]    = INPUT_KEY_UP,
    [SDL_SCANCODE_DOWN]  = INPUT_KEY_DOWN,
    [SDL_SCANCODE_LEFT]  = INPUT_KEY_LEFT,
    [SDL_SCANCODE_RIGHT] = INPUT_KEY_RIGHT,
};
static int32_t input_map_native_scancode_to_input_keycode(int32_t native_scancode) {
    uint32_t mapping = _sdl_scancode_to_input_keycode_table[native_scancode];

    if (mapping == INPUT_KEY_UNKNOWN) {
        printf("no key mapping? (%d)\n", native_scancode);
    }

    return mapping;
}

void blackiron_monkey_host_request_render_limit_preferences(struct graphics_context_limits* output);
void blackiron_monkey_host_initialize(struct host_state* state, int argument_count, char** argument_values);
void blackiron_monkey_host_frame(struct host_state* state, float delta_time);
void blackiron_monkey_host_deinitialize(struct host_state* state);

void* platform_opengl_load_function(const char* name) {
    return SDL_GL_GetProcAddress(name);
}

static void _sdl_update_graphics_context_dimensions(SDL_Window* window, struct graphics_context* context) {
    int32_t new_screen_width;
    int32_t new_screen_height;

    SDL_GL_GetDrawableSize(window, &new_screen_width, &new_screen_height);
    graphics_context_update_screen_dimensions(context, new_screen_width, new_screen_height);
}

static SDL_GameController* _sdl_controller_devices[4] = {};
void input_poll_for_new_controllers(struct input* input) {
    input->current.controller_count = 0;

    // NOTE(jerry):
    // this is probably inefficient in comparison to the
    // event version. But this maps the closest to the XInput code in the windows
    // version for now.

    // Register devices here.
    for (unsigned controller_device_index = 0; controller_device_index < 4; ++controller_device_index) {
        SDL_GameController* current_controller_device = _sdl_controller_devices[controller_device_index];

        if (current_controller_device) {
            if (SDL_GameControllerGetAttached(current_controller_device)) {
                continue;
            } else {
                SDL_GameControllerClose(current_controller_device);
                _sdl_controller_devices[controller_device_index] = NULL;
            }
        } else {
            SDL_GameController* new_device = SDL_GameControllerOpen(controller_device_index);

            if (new_device && SDL_GameControllerGetAttached(new_device)) {
                _sdl_controller_devices[controller_device_index] = new_device;
            } else {
                _sdl_controller_devices[controller_device_index] = NULL;
                SDL_GameControllerClose(new_device);
            }
        }
    }

    // Assign controller states to these ids so we can start harvesting data later.
    for (unsigned user_index = 0; user_index < 4; ++user_index) {
        if (_sdl_controller_devices[user_index]) {
            struct controller_input_state* controller = &input->current.controllers[input->current.controller_count++];
            controller->id = user_index;
        }
    }
}

static void _sdl_input_update_controller(struct input* input, uint8_t index) {
    struct controller_input_state* current_controller = &input->current.controllers[index];
    SDL_GameController* cooresponding_controller_device = _sdl_controller_devices[current_controller->id];

    {
        current_controller->left_trigger_pressure  = SDL_GameControllerGetAxis(cooresponding_controller_device, SDL_CONTROLLER_AXIS_TRIGGERLEFT);
        current_controller->right_trigger_pressure = SDL_GameControllerGetAxis(cooresponding_controller_device, SDL_CONTROLLER_AXIS_TRIGGERRIGHT);
    }

    {
        current_controller->buttons[INPUT_CONTROLLER_A] = SDL_GameControllerGetButton(cooresponding_controller_device, SDL_CONTROLLER_BUTTON_A);
        current_controller->buttons[INPUT_CONTROLLER_B] = SDL_GameControllerGetButton(cooresponding_controller_device, SDL_CONTROLLER_BUTTON_B);
        current_controller->buttons[INPUT_CONTROLLER_X] = SDL_GameControllerGetButton(cooresponding_controller_device, SDL_CONTROLLER_BUTTON_X);
        current_controller->buttons[INPUT_CONTROLLER_Y] = SDL_GameControllerGetButton(cooresponding_controller_device, SDL_CONTROLLER_BUTTON_Y);
    }

    {
        current_controller->buttons[INPUT_CONTROLLER_DPAD_UP]    = SDL_GameControllerGetButton(cooresponding_controller_device, SDL_CONTROLLER_BUTTON_DPAD_UP);
        current_controller->buttons[INPUT_CONTROLLER_DPAD_DOWN]  = SDL_GameControllerGetButton(cooresponding_controller_device, SDL_CONTROLLER_BUTTON_DPAD_DOWN);
        current_controller->buttons[INPUT_CONTROLLER_DPAD_LEFT]  = SDL_GameControllerGetButton(cooresponding_controller_device, SDL_CONTROLLER_BUTTON_DPAD_LEFT); 
        current_controller->buttons[INPUT_CONTROLLER_DPAD_RIGHT] = SDL_GameControllerGetButton(cooresponding_controller_device, SDL_CONTROLLER_BUTTON_DPAD_RIGHT); 
    }

    {
        current_controller->buttons[INPUT_CONTROLLER_RIGHT_THUMB] = SDL_GameControllerGetButton(cooresponding_controller_device, SDL_CONTROLLER_BUTTON_RIGHTSTICK);
        current_controller->buttons[INPUT_CONTROLLER_LEFT_THUMB]  = SDL_GameControllerGetButton(cooresponding_controller_device, SDL_CONTROLLER_BUTTON_LEFTSTICK);
    }

    {
        current_controller->buttons[INPUT_CONTROLLER_START] = SDL_GameControllerGetButton(cooresponding_controller_device, SDL_CONTROLLER_BUTTON_START);
        current_controller->buttons[INPUT_CONTROLLER_BACK]  = SDL_GameControllerGetButton(cooresponding_controller_device, SDL_CONTROLLER_BUTTON_BACK);
    }

    {
        current_controller->left_thumbstick = (struct controller_thumbstick_state) {
            .x = SDL_GameControllerGetAxis(cooresponding_controller_device, SDL_CONTROLLER_AXIS_LEFTX),
            .y = -SDL_GameControllerGetAxis(cooresponding_controller_device, SDL_CONTROLLER_AXIS_LEFTY),
        };
        current_controller->right_thumbstick = (struct controller_thumbstick_state) {
            .x = SDL_GameControllerGetAxis(cooresponding_controller_device, SDL_CONTROLLER_AXIS_RIGHTX),
            .y = -SDL_GameControllerGetAxis(cooresponding_controller_device, SDL_CONTROLLER_AXIS_RIGHTY),
        };
    }
}

void input_update_controller_states(struct input* input) {
    // NOTE(jerry): have a platform flag to state if this is a direct input or xinput device.
    for (unsigned controller_index = 0; controller_index < input->current.controller_count; ++controller_index) {
        struct controller_input_state* current_controller = &input->current.controllers[controller_index];

        // NOTE(jerry): MSDN says you should check the packet state for changes.
        // I'm just going to continuously update since it's probably negligable.

        if (_sdl_controller_devices[controller_index]) {
            _sdl_input_update_controller(input, controller_index);
        } else {
            // How does one pull a controller out with sub-frame accuracy?
            // this would thoroughly interest me if it can be done consistently.

            // NOTE(jerry): Okay... turns out it's not nearly as impossible as I thought it would be.
            // this isn't a critical error, though so I'll have to devise a better way to handle this...
            // Which might just be the index->id table or something like that.

            // unreachable_code();
        }
    }
}

// Should all of these be pointers? Probably
// not, but I'm not really going to spend time architecting this right now.
struct blackiron_main_loop_packet {
    // We don't need to modify the window, so we don't need a pointer to pointer.
    SDL_Window* application_window_handle;

    struct host_application_description* application;
    struct host_state* host_state;

    uint64_t* start_time;
    uint64_t* end_time;
    float*    last_delta_time;
};

static void _blackiron_main_loop(void* user_data) {
    // reset per frame allocator, so we can use it without suspecting anything going wrong!
    // Ever!
    _global_temporary_allocator.used = 0;

    struct blackiron_main_loop_packet* parameters = user_data;

    SDL_Event event;
    // ... I'm writing this in vim, and it's autoindent settings don't match my Emacs setup...
    // FUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_WINDOWEVENT: {
                switch (event.window.event) {
                    case SDL_WINDOWEVENT_CLOSE:
                    case SDL_WINDOWEVENT_FOCUS_LOST:
                    case SDL_WINDOWEVENT_FOCUS_GAINED:
                    case SDL_WINDOWEVENT_LEAVE:
                    case SDL_WINDOWEVENT_ENTER:
                    case SDL_WINDOWEVENT_RESTORED:
                    case SDL_WINDOWEVENT_MAXIMIZED:
                    case SDL_WINDOWEVENT_MINIMIZED:
                    case SDL_WINDOWEVENT_MOVED:
                    case SDL_WINDOWEVENT_EXPOSED:
                    case SDL_WINDOWEVENT_HIDDEN:
                    case SDL_WINDOWEVENT_SHOWN: {
                    } break;
                    case SDL_WINDOWEVENT_RESIZED:
                    case SDL_WINDOWEVENT_SIZE_CHANGED: {
                        _sdl_update_graphics_context_dimensions(parameters->application_window_handle, &application_graphics_context);
                    } break;
                }
            } break;

            case SDL_QUIT: {
                application_quit = true;
            } break;

            case SDL_TEXTINPUT: {
                SDL_TextInputEvent text_event = event.text;
                bool* keystate = application_input.current.keys.down;

                // NOTE(jerry): IME??
                // I mean this is kind of using SDL2 wrong, but whatever.
                // Don't allow send_character events when we have modifier keys
                if (keystate[INPUT_KEY_ALT] == false && keystate[INPUT_KEY_CTRL] == false) {
                    console_send_character(text_event.text[0]);
                }
            } break;

            case SDL_KEYUP:
            case SDL_KEYDOWN: {
                // NOTE(jerry):
                // does not use send_key_inputs, since it has to special case certain behaviors
                // since the way I identify key input on win32 is not the same as SDL2.
                SDL_KeyboardEvent keyboard_event = event.key;
                int32_t remapped_key_code_from_scancode = input_map_native_scancode_to_input_keycode(keyboard_event.keysym.scancode);
                application_input.current.keys.down[remapped_key_code_from_scancode] = (keyboard_event.type == SDL_KEYDOWN);

                if (keyboard_event.keysym.sym == SDLK_BACKSPACE && keyboard_event.type == SDL_KEYDOWN) {
                    console_send_character('\b');
                }

                bool* keystate = application_input.current.keys.down;
                console_send_key_event((keyboard_event.type == SDL_KEYDOWN), remapped_key_code_from_scancode, keystate[INPUT_KEY_ALT], keystate[INPUT_KEY_CTRL], keystate[INPUT_KEY_SHIFT]);
            } break;

            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP: {
                SDL_MouseButtonEvent button_event = event.button;
                uint32_t mouse_state = SDL_GetMouseState(0,0);

                platform_send_mouse_inputs(button_event.x, button_event.y, (mouse_state & SDL_BUTTON(SDL_BUTTON_LEFT)) > 0, (mouse_state & SDL_BUTTON(SDL_BUTTON_MIDDLE)) > 0, (mouse_state & SDL_BUTTON(SDL_BUTTON_RIGHT))  > 0);
            } break;

            case SDL_MOUSEMOTION: {
                SDL_MouseMotionEvent motion_event = event.motion;
                platform_send_mouse_position(motion_event.x, motion_event.y);

                _intentionally_unused(motion_event.xrel);
                _intentionally_unused(motion_event.yrel);
            } break;

            case SDL_CONTROLLERDEVICEREMOVED:
            case SDL_CONTROLLERDEVICEADDED: {
                input_poll_for_new_controllers(&application_input);
            } break;
        }
    }

    input_update_controller_states(&application_input);

    platform_frame(parameters->application, parameters->host_state, *parameters->last_delta_time);

    SDL_GL_SetSwapInterval(0);
    SDL_GL_SwapWindow(parameters->application_window_handle);

    *parameters->end_time = SDL_GetTicks();
    uint64_t elapsed_time = *parameters->end_time - *parameters->start_time;
    *parameters->last_delta_time = (elapsed_time / 1000.0f);
    *parameters->start_time = *parameters->end_time;
}

int blackiron_main(int argument_count, char** argument_values) {
    __testing_basic_read();
    // blam
    SDL_GLContext opengl_context;
    SDL_Init(SDL_INIT_EVERYTHING);
    {
        SDL_Window* application_window_handle = SDL_CreateWindow("Blackiron Monkey [SDL2]", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL);

        global_window_handle = application_window_handle;


        {
            SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
            SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
            SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);

            SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
            SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 16);
            SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

#if __EMSCRIPTEN__
            /*
              It appears emscripten seems to dynamically check for what opengl it can use...
              
              Gratefully this only took 30 minutes to find out.

              so it's either core or legacy, no compatability. Shucks.
             */

            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#else
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, BLACKIRON_OPENGL_MAJOR_VERSION);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, BLACKIRON_OPENGL_MINOR_VERSION);

#ifdef BLACKIRON_OPENGL_COMPATIBILITY
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
#else
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#endif
#endif

            opengl_context = SDL_GL_CreateContext(application_window_handle);
            platform_load_opengl_function_pointers();
            SDL_GL_MakeCurrent(application_window_handle, opengl_context);
        }

        uint64_t start_time = SDL_GetTicks();
        uint64_t end_time;

        float last_delta_time = (1/60.0f);

        struct host_application_description application;
        struct host_state host_state = {};

        platform_initialize(&application, &host_state, argument_count, argument_values);
        _sdl_update_graphics_context_dimensions(application_window_handle, &application_graphics_context);

#if __EMSCRIPTEN__
        console_printf("EMSCRIPTEN MODE\n");
        struct blackiron_main_loop_packet packet = (struct blackiron_main_loop_packet){
            .application_window_handle = application_window_handle,
            .application = &application,
            .host_state = &host_state,
            .start_time = &start_time,
            .end_time = &end_time,
            .last_delta_time = &last_delta_time
        };
        emscripten_set_main_loop_arg(_blackiron_main_loop, &packet, 0, 1);
#else
        while (!application_quit) {
            _blackiron_main_loop(
                    &(struct blackiron_main_loop_packet) {
                        .application_window_handle = application_window_handle,
                        .application = &application,
                        .host_state = &host_state,
                        .start_time = &start_time,
                        .end_time = &end_time,
                        .last_delta_time = &last_delta_time
                    });
        }
#endif
        platform_deinitialize(&application, &host_state);
    }

    {
        for (unsigned controller_device_index = 0; controller_device_index < 4; ++controller_device_index) {
            if (_sdl_controller_devices[controller_device_index]) {
                SDL_GameControllerClose(_sdl_controller_devices[controller_device_index]);
            }
        }
    }

    SDL_GL_DeleteContext(opengl_context);
    SDL_DestroyWindow(global_window_handle);

    SDL_Quit();
    return 0;
}
