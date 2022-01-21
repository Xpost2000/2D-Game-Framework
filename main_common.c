char _game_dll_name[256] = {};

// function signatures forward declared for stuff later.
void* platform_load_shared_object(char* file_location);
void* platform_load_function_from_shared_object(void* shared_object, char* function_name);
void platform_unload_shared_object(void* handle);

static void _handle_command_line_arguments(int argument_count, char** argument_values) {
#if !BLACKIRON_STATIC_EXECUTABLE
    if (argument_count == 2) {
        strncpy(_game_dll_name, argument_values[1], 255);
        printf("Will be loading \"%s\" as a game! Hopefully this works.\n", _game_dll_name);
    }
#endif
    // TODO(jerry): more advanced handling?
}

void* platform_opengl_load_function(const char*);
static void platform_load_opengl_function_pointers(void) {
    #if 0
    if (!gladLoadGL(platform_opengl_load_function)) {
        printf("Yikes! Something went wrong chief!\n");
        return;
    }
    #else
    if (glewInit() != GLEW_OK) {
        printf("Yikes! Something went wrong chief!\n");
        return;
    }
    #endif
}

void host_function_stub_initialize(struct host_state* _1, int _2, char** _3) {}
void host_function_stub_query_render_preferences(struct graphics_context_limits* _1) {}
void host_function_stub_frame(struct host_state* _1, float _2) {
    _1->graphics->clear_buffer(0.0, 0.0, 0.0, 1.0);
}
void host_function_stub_fixed_frame(struct host_state* _1, float _2) {}
void host_function_stub_deinitialize(struct host_state* _1) {}

static struct host_api host_function_stubs = {
    .initialize               = host_function_stub_initialize,
    .query_render_preferences = host_function_stub_query_render_preferences,
    .frame                    = host_function_stub_frame,
    .fixed_frame              = host_function_stub_fixed_frame,
    .deinitialize             = host_function_stub_deinitialize,
};

struct host_application_description host_function_stub_get_application_description(void) {
    return (struct host_application_description) {
        .functions = host_function_stubs,
    };
}

static void platform_initialize(struct host_application_description* application_description, struct host_state* host_state, int argument_count, char** argument_values) {
    file_hentai_initialize();

    shared_object_get_application_description_function get_application_description_function = host_function_stub_get_application_description;
#ifdef BLACKIRON_STATIC_EXECUTABLE
    get_application_description_function = get_application_description;
#else
    {
        void* game = platform_load_shared_object(_game_dll_name);
        fprintf(stderr, "(%s) %p\n", _game_dll_name, game);
        if (game) {
            get_application_description_function = platform_load_function_from_shared_object(game, "get_application_description");
        }
    }
#endif

    *application_description = get_application_description_function();
    if (get_application_description_function == host_function_stub_get_application_description) {
        printf("Engine was probably built to only dynamically load games.\nPlease feed me a dll!\n\"%s\" will be searched for first!\nFor now have a blank screen!", BLACKIRON_DEFAULT_GAME_DLL_NAME);
    } else {
        // check all function pointers and replace them with stubs if bad so
        // we don't crash from calling a null function.
        {
            if (!application_description->functions.initialize) {
                application_description->functions.initialize = host_function_stubs.initialize;
                console_printf("missing initialize function!\n");
            }
            if (!application_description->functions.query_render_preferences) {
                application_description->functions.query_render_preferences = host_function_stubs.query_render_preferences;
                console_printf("missing query_render_preferences function!\n");
            }
            if (!application_description->functions.frame) {
                application_description->functions.frame = host_function_stubs.frame;
                console_printf("missing frame function!\n");
            }
            if (!application_description->functions.fixed_frame) {
                application_description->functions.fixed_frame = host_function_stubs.fixed_frame;
                console_printf("missing fixed_frame function!\n");
            }
            if (!application_description->functions.deinitialize) {
                application_description->functions.deinitialize = host_function_stubs.deinitialize;
                console_printf("missing deinitialize function!\n");
            }
        }
    }


    // initialize graphics_context here.
    struct graphics_context_limits limits = graphics_context_default_limits;

    application_description->functions.query_render_preferences(&limits);
    graphics_context_initialize(&application_graphics_context, limits);
    audio_open_device();
    populate_all_apis(host_state);
    console_initialize(&_global_graphics_context_api);

    application_description->functions.initialize(host_state, argument_count, argument_values);

    // analyze the application descriptor and do things.
    {
        // function prototypes declared here
        // to avoid weird declaration formatting in platform files...
        void platform_set_window_title(char*);
        void platform_set_window_resolution(int32_t, int32_t);
        void platform_set_window_resizable(bool);
        void platform_set_window_fullscreen(bool);

        if (application_description->title) {
            platform_set_window_title(application_description->title);
        }

        if (application_description->window.width > 0 && application_description->window.height > 0) {
            platform_set_window_resolution(application_description->window.width, application_description->window.height);
        } else {
            if (application_description->window.width > 0 || application_description->window.height > 0) {
                console_printf("? was this a mistake? Resolution was not set because either width or height were zero for some reason... Check your init?\n");
            }
        }

        platform_set_window_resizable(application_description->window.resizable);
        platform_set_window_fullscreen(application_description->window.fullscreen);
    }

    console_printf("==== Engine Start ====\n");
    console_printf("Welcome to BlackIron Monkey!\n");
    console_printf("A simple C 2D framework / engine or thing.\n");
}

static void platform_send_text_input(uint32_t codepoint) {
    console_send_character(codepoint);
}

static void platform_send_mouse_position(int32_t mouse_x, int32_t mouse_y) {
    application_input.current.mouse.x = mouse_x;
    application_input.current.mouse.y = mouse_y;
}
static void platform_send_mouse_inputs(int32_t mouse_x, int32_t mouse_y, bool mouse_left, bool mouse_middle, bool mouse_right) {
    application_input.current.mouse.x = mouse_x;
    application_input.current.mouse.y = mouse_y;

    application_input.current.mouse.buttons[MOUSE_BUTTON_LEFT]   = mouse_left;
    application_input.current.mouse.buttons[MOUSE_BUTTON_MIDDLE] = mouse_middle;
    application_input.current.mouse.buttons[MOUSE_BUTTON_RIGHT]  = mouse_right;
}

static void platform_send_key_inputs(bool is_key_down, int32_t scancode_value, int32_t virtual_keycode_value) {
    application_input.current.keys.down[scancode_value] = is_key_down;

    bool* keystate = application_input.current.keys.down;
    if (keystate[INPUT_KEY_ALT] && keystate[INPUT_KEY_F4]) {
        application_quit = true;
    }

    console_send_key_event(is_key_down, scancode_value, keystate[INPUT_KEY_ALT], keystate[INPUT_KEY_CTRL], keystate[INPUT_KEY_SHIFT]);
}

static void platform_frame(struct host_application_description* application_description, struct host_state* host_state, float last_delta_time) {
    // fixed update
    if ((int)target_framerate > 0)  {
        while (frametime_accumulator >= target_inverse_framerate) {
            frametime_accumulator -= target_inverse_framerate;
        }
    } else {
        application_description->functions.fixed_frame(host_state, last_delta_time);
    }

    // non fixed update.
    application_description->functions.frame(host_state, last_delta_time);
    console_frame(&_global_graphics_context_api, &_global_input_api, last_delta_time);
    file_hentai_update(last_delta_time);
    audio_update(last_delta_time);

    graphics_context_update(&application_graphics_context, last_delta_time);

    // double buffered input.
    application_input.last = application_input.current;
    /* global */frametime_accumulator += last_delta_time;
    framerate_sampler_update(last_delta_time);
}

static void platform_deinitialize(struct host_application_description* application_description, struct host_state* host_state) {
    audio_close_device();
    file_hentai_deinitialize();
    application_description->functions.deinitialize(host_state);
    graphics_context_deinitialize(&application_graphics_context);

    _system_memory_leak_check();
}
