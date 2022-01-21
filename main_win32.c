// NOTE(jerry):
// It might be in my best interest to get a Visual Studio project
// or find a decent gdb / lldb graphical debugger
//
// just in the event I find problems I legitimately can't debug
// because for as anachronistic my programming style is
// even I don't wish to be debugging like it's 1970.
//

// TODO(jerry):
// Consider splitting common parts of the platform layers out! Since now I can obviously
// see some similar things. That it would be worthwhile to do so.

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef far
#undef near

#include <xinput.h>
#include "platform_prelude.c"

#include <bundled_external/glext.h>
#include <bundled_external/wglext.h>

void* platform_load_shared_object(char* file_location) {
    HMODULE handle = LoadLibrary(file_location);
    if (!handle) {
        fprintf(stderr, "error in load library, give better message later\n");
        return NULL;
    }
    return handle;
}
void* platform_load_function_from_shared_object(void* shared_object, char* function_name) {
    void* procedure_address = GetProcAddress((HMODULE)shared_object, function_name);
    return procedure_address;
}
void platform_unload_shared_object(void* handle) {
    FreeLibrary((HMODULE)handle);
}

static HWND global_window_handle;

void platform_set_window_title(char* title) {
    SetWindowText(global_window_handle, title);
}
void platform_set_window_resolution(int32_t width, int32_t height) {
    RECT window_rect;
    GetWindowRect(global_window_handle, &window_rect);

    {
        RECT desired_client_area_rectangle = (RECT) {
            .left = 0,
            .top = 0,

            .right = width,
            .bottom = height,
        };

        LONG v = GetWindowLongPtr(global_window_handle, GWL_STYLE);
        AdjustWindowRect(&desired_client_area_rectangle, v, false);

        width  = desired_client_area_rectangle.right - desired_client_area_rectangle.left;
        height = desired_client_area_rectangle.bottom - desired_client_area_rectangle.top;
    }

    // grrr.
    SetWindowPos(global_window_handle, HWND_TOP, window_rect.left, window_rect.top, width, height, 0);
    UpdateWindow(global_window_handle);
}
void platform_set_window_resizable(bool value) {
    LONG v = GetWindowLongPtr(global_window_handle, GWL_STYLE);

    bool has_thickframe  = v & WS_THICKFRAME;
    bool has_maximizebox = v & WS_MAXIMIZEBOX;

    if (value) {
        if (has_maximizebox) {
            v |= WS_MAXIMIZEBOX;
        }

        if (has_thickframe) {
            v |= WS_THICKFRAME;
        }
    } else {
        if (has_maximizebox) {
            v &= ~WS_MAXIMIZEBOX;
        }

        if (has_thickframe) {
            v &= ~WS_THICKFRAME;
        }
    }

    SetWindowLongPtr(global_window_handle, GWL_STYLE, v);
}

// Grrr.. bad win32 code but whatever.
// Why does this have to be so verbose with the win32 API?
// NOTE(jerry): This isn't exclusive fullscreen! This is just borderless fullscreen
// which is honestly probably the best default even if it's less performant.
void platform_set_window_fullscreen(bool value) {
    static WINDOWPLACEMENT last_non_fullscreen_window_placement = {};

    LONG style = GetWindowLong(global_window_handle, GWL_STYLE);
    bool has_overlapped_window = style & WS_OVERLAPPEDWINDOW;

    if (has_overlapped_window) {
        if (value) {
            MONITORINFO monitor_info = {.cbSize = sizeof(monitor_info)};
            WINDOWPLACEMENT window_placement;

            if (GetWindowPlacement(global_window_handle, &window_placement) &&
                GetMonitorInfo(MonitorFromWindow(global_window_handle, MONITOR_DEFAULTTOPRIMARY), &monitor_info)) {
                last_non_fullscreen_window_placement = window_placement;

                SetWindowLong(global_window_handle, GWL_STYLE, style & ~WS_OVERLAPPEDWINDOW);
                SetWindowPos(global_window_handle, HWND_TOP,
                             monitor_info.rcMonitor.left, monitor_info.rcMonitor.top,
                             monitor_info.rcMonitor.right - monitor_info.rcMonitor.left,
                             monitor_info.rcMonitor.bottom - monitor_info.rcMonitor.top,
                             SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
            }
        }
    } else {
        if (!value) {
            SetWindowLong(global_window_handle, GWL_STYLE, style | WS_OVERLAPPEDWINDOW); 
            SetWindowPos(global_window_handle, 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
            SetWindowPlacement(global_window_handle, &last_non_fullscreen_window_placement);
        }
    }
}

// TODO(jerry): Neither table supports numpad keys.
static uint8_t _win32_scancode_to_input_keycode_table[255] = {
    // top row
    [1] = INPUT_KEY_ESCAPE,
    [2] = INPUT_KEY_1,
    [3] = INPUT_KEY_2,
    [4] = INPUT_KEY_3,
    [5] = INPUT_KEY_4,
    [6] = INPUT_KEY_5,
    [7] = INPUT_KEY_6,
    [8] = INPUT_KEY_7,
    [9] = INPUT_KEY_8,
    [10] = INPUT_KEY_9,
    [11] = INPUT_KEY_0,
    [12] = INPUT_KEY_MINUS,
    [13] = INPUT_KEY_EQUALS,
    [14] = INPUT_KEY_BACKSPACE,

    [15] = INPUT_KEY_TAB,

    [16] = INPUT_KEY_Q,
    [17] = INPUT_KEY_W,
    [18] = INPUT_KEY_E,
    [19] = INPUT_KEY_R,
    [20] = INPUT_KEY_T,
    [21] = INPUT_KEY_Y,
    [22] = INPUT_KEY_U,
    [23] = INPUT_KEY_I,
    [24] = INPUT_KEY_O,
    [25] = INPUT_KEY_P,
    [26] = INPUT_KEY_LEFT_BRACKET,
    [27] = INPUT_KEY_RIGHT_BRACKET,
    [28] = INPUT_KEY_RETURN,

    // using scancodes it is actually possible to differentiate left and right control.
    // but I don't care about that.
    [29] = INPUT_KEY_CTRL, // left control (interesting that this is here, and not caps lock)

    [30] = INPUT_KEY_A,
    [31] = INPUT_KEY_S,
    [32] = INPUT_KEY_D,
    [33] = INPUT_KEY_F,
    [34] = INPUT_KEY_G,
    [35] = INPUT_KEY_H,
    [36] = INPUT_KEY_J,
    [37] = INPUT_KEY_K,
    [38] = INPUT_KEY_L,
    [39] = INPUT_KEY_SEMICOLON,
    [40] = INPUT_KEY_QUOTE,
    [41] = INPUT_KEY_BACKQUOTE,

    [42] = INPUT_KEY_SHIFT, // left shift
    [43] = INPUT_KEY_BACKSLASH,

    [44] = INPUT_KEY_Z,
    [45] = INPUT_KEY_X,
    [46] = INPUT_KEY_C,
    [47] = INPUT_KEY_V,
    [48] = INPUT_KEY_B,
    [49] = INPUT_KEY_N,
    [50] = INPUT_KEY_M,
    [51] = INPUT_KEY_COMMA,
    [52] = INPUT_KEY_PERIOD,
    [53] = INPUT_KEY_FORWARDSLASH,

    [54] = INPUT_KEY_SHIFT, // right shift
    [55] = INPUT_KEY_PRINTSCREEN,
    [56] = INPUT_KEY_ALT,
    [57] = INPUT_KEY_SPACE,
    //[58] = INPUT_KEY_CAPSLOCK,
    [59] = INPUT_KEY_F1,
    [60] = INPUT_KEY_F2,
    [61] = INPUT_KEY_F3,
    [62] = INPUT_KEY_F4,
    [63] = INPUT_KEY_F5,
    [64] = INPUT_KEY_F6,
    [65] = INPUT_KEY_F7,
    [66] = INPUT_KEY_F8,
    [67] = INPUT_KEY_F9,
    [68] = INPUT_KEY_F10,

    [69] = INPUT_KEY_NUMBER_LOCK,
    [70] = INPUT_KEY_SCROLL_LOCK,

    [86] = INPUT_KEY_F11,
    [87] = INPUT_KEY_F12,

    [73] = INPUT_KEY_PAGEUP,
    [71] = INPUT_KEY_HOME,
    [81] = INPUT_KEY_PAGEDOWN,
    [82] = INPUT_KEY_INSERT,
    [83] = INPUT_KEY_DELETE,
    [79] = INPUT_KEY_END,

    [75] = INPUT_KEY_LEFT,
    [72] = INPUT_KEY_UP,
    [77] = INPUT_KEY_RIGHT,
    [80] = INPUT_KEY_DOWN,
};
static uint8_t _win32_virtual_keycode_to_input_keycode_table[255] = {
    [VK_BACK] = INPUT_KEY_BACKSPACE,
    [VK_TAB]  = INPUT_KEY_TAB,
    [VK_RETURN] = INPUT_KEY_RETURN,
    [VK_SHIFT] = INPUT_KEY_SHIFT,
    [VK_CONTROL] = INPUT_KEY_CTRL,
    [VK_SPACE] = INPUT_KEY_SPACE,
    [VK_MENU] = INPUT_KEY_ALT,

    ['0'] = INPUT_KEY_0,
    ['1'] = INPUT_KEY_1,
    ['2'] = INPUT_KEY_2,
    ['3'] = INPUT_KEY_3,
    ['4'] = INPUT_KEY_4,
    ['5'] = INPUT_KEY_5,
    ['6'] = INPUT_KEY_6,
    ['7'] = INPUT_KEY_7,
    ['8'] = INPUT_KEY_8,
    ['9'] = INPUT_KEY_9,

    ['A'] = INPUT_KEY_A,
    ['B'] = INPUT_KEY_B,
    ['C'] = INPUT_KEY_C,
    ['D'] = INPUT_KEY_D,
    ['E'] = INPUT_KEY_E,
    ['F'] = INPUT_KEY_F,
    ['G'] = INPUT_KEY_G,
    ['H'] = INPUT_KEY_H,
    ['I'] = INPUT_KEY_I,
    ['J'] = INPUT_KEY_J,
    ['K'] = INPUT_KEY_K,
    ['L'] = INPUT_KEY_L,
    ['M'] = INPUT_KEY_M,
    ['N'] = INPUT_KEY_N,
    ['O'] = INPUT_KEY_O,
    ['P'] = INPUT_KEY_P,
    ['Q'] = INPUT_KEY_Q,
    ['R'] = INPUT_KEY_R,
    ['S'] = INPUT_KEY_S,
    ['T'] = INPUT_KEY_T,
    ['U'] = INPUT_KEY_U,
    ['V'] = INPUT_KEY_V,
    ['W'] = INPUT_KEY_W,
    ['X'] = INPUT_KEY_X,
    ['Y'] = INPUT_KEY_Y,
    ['Z'] = INPUT_KEY_Z,

    [191] = INPUT_KEY_BACKSLASH,
    [VK_OEM_102] = INPUT_KEY_FORWARDSLASH,
    [VK_OEM_COMMA] = INPUT_KEY_COMMA,
    [222] = INPUT_KEY_QUOTE,
    [VK_OEM_1] = INPUT_KEY_SEMICOLON,
    [VK_OEM_PERIOD] = INPUT_KEY_PERIOD,

    [45] = INPUT_KEY_INSERT,
    [36] = INPUT_KEY_HOME,
    [33] = INPUT_KEY_PAGEUP,
    [34] = INPUT_KEY_PAGEDOWN,
    [46] = INPUT_KEY_DELETE,
    [35] = INPUT_KEY_END,

    [189] = INPUT_KEY_MINUS,
    [187] = INPUT_KEY_EQUALS,
    [44]  = INPUT_KEY_PRINTSCREEN,

    [219] = INPUT_KEY_LEFT_BRACKET,
    [221] = INPUT_KEY_RIGHT_BRACKET,
    [192] = INPUT_KEY_BACKQUOTE,

    [112] = INPUT_KEY_F1,
    [113] = INPUT_KEY_F2,
    [114] = INPUT_KEY_F3,
    [115] = INPUT_KEY_F4,
    [116] = INPUT_KEY_F5,
    [117] = INPUT_KEY_F6,
    [118] = INPUT_KEY_F7,
    [119] = INPUT_KEY_F8,
    [120] = INPUT_KEY_F9,
    [121] = INPUT_KEY_F10,
    [122] = INPUT_KEY_F11,
    [123] = INPUT_KEY_F12,

    [19] = INPUT_KEY_PAUSE,
    [145] = INPUT_KEY_SCROLL_LOCK,
    [144] = INPUT_KEY_NUMBER_LOCK,
};
static int32_t input_map_native_keycode_to_input_keycode(int32_t native_keycode) {
    uint32_t mapping = _win32_virtual_keycode_to_input_keycode_table[native_keycode];

#if 0
    if (mapping == INPUT_KEY_UNKNOWN) {
        fprintf(stderr, "no key mapping? (%d)\n", native_keycode);
    }
#endif

    return mapping;
}

static int32_t input_map_native_scancode_to_input_keycode(int32_t native_scancode) {
    uint32_t mapping = _win32_scancode_to_input_keycode_table[native_scancode];

#if 0
    if (mapping == INPUT_KEY_UNKNOWN) {
        fprintf(stderr, "no key mapping? (%d)\n", native_keycode);
    }
#endif

    return mapping;
}

void input_poll_for_new_controllers(struct input* input) {
    input->current.controller_count = 0;

    // NOTE(jerry): Attempt to do direct input polling first.

    // NOTE(jerry):
    // this shouldn't fuck anything up.
    // if it does, I'll just use an id->index lookup table to stop that then.

    for (unsigned user_index = 0; user_index < 4; ++user_index) {
        // I'm not actually going to sync the state. I'm really
        // just polling here. I sync later.
        XINPUT_STATE xinput_state = {0};
        DWORD        connected    = XInputGetState(user_index, &xinput_state);

        // NOTE(jerry): load hit store probably happening
        if (connected == ERROR_SUCCESS) {
            struct controller_input_state* controller = &input->current.controllers[input->current.controller_count++];
            controller->id = user_index;
        }
    }
}

void input_update_controller_states(struct input* input) {
    // NOTE(jerry): have a platform flag to state if this is a direct input or xinput device.
    for (unsigned controller_index = 0; controller_index < input->current.controller_count; ++controller_index) {
        struct controller_input_state* current_controller = &input->current.controllers[controller_index];

        // NOTE(jerry): MSDN says you should check the packet state for changes.
        // I'm just going to continuously update since it's probably negligable.
        XINPUT_STATE xinput_state = {0};
        DWORD        connected    = XInputGetState(current_controller->id, &xinput_state);

        if (connected == ERROR_SUCCESS) {
            XINPUT_GAMEPAD gamepad = xinput_state.Gamepad;
            // NOTE(jerry): XINPUT_GAMEPAD_LEFT_HUMB_DEADZONE
            current_controller->left_thumbstick = (struct controller_thumbstick_state) {
                .x =  gamepad.sThumbLX,
                .y =  gamepad.sThumbLY,
            };
            current_controller->right_thumbstick = (struct controller_thumbstick_state) {
                .x = gamepad.sThumbRX,
                .y = gamepad.sThumbRY,
            };

            current_controller->left_trigger_pressure  = gamepad.bLeftTrigger;
            current_controller->right_trigger_pressure = gamepad.bRightTrigger;

            current_controller->buttons[INPUT_CONTROLLER_A] = (gamepad.wButtons & XINPUT_GAMEPAD_A);
            current_controller->buttons[INPUT_CONTROLLER_B] = (gamepad.wButtons & XINPUT_GAMEPAD_B);
            current_controller->buttons[INPUT_CONTROLLER_X] = (gamepad.wButtons & XINPUT_GAMEPAD_X);
            current_controller->buttons[INPUT_CONTROLLER_Y] = (gamepad.wButtons & XINPUT_GAMEPAD_Y);

            current_controller->buttons[INPUT_CONTROLLER_DPAD_DOWN]  = (gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
            current_controller->buttons[INPUT_CONTROLLER_DPAD_LEFT]  = (gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
            current_controller->buttons[INPUT_CONTROLLER_DPAD_UP]    = (gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP);
            current_controller->buttons[INPUT_CONTROLLER_DPAD_RIGHT] = (gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);

            current_controller->buttons[INPUT_CONTROLLER_RIGHT_THUMB] = (gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB);
            current_controller->buttons[INPUT_CONTROLLER_LEFT_THUMB]  = (gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB);

            current_controller->buttons[INPUT_CONTROLLER_START] = (gamepad.wButtons & XINPUT_GAMEPAD_START);
            current_controller->buttons[INPUT_CONTROLLER_BACK]  = (gamepad.wButtons & XINPUT_GAMEPAD_BACK);
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
// end input

// The only reason this is statically allocated is for the function
// as it is semi error prone to inline this every single time.
static size_t _argument_values_allocation_buffer_allocated = 0;
static char   _argument_values_allocation_buffer[MAX_ARGUMENT_VALUE_STRING_POOL_SIZE]     = {0};
static char* argument_values_add_string(const char* string, size_t string_size) {
    char* base_pointer = _argument_values_allocation_buffer + _argument_values_allocation_buffer_allocated;
    memcpy(base_pointer, string, string_size);

    // This is a little finky because one of the worst things in the world are null-terminated strings
    // unfortunately the posix compliant main requires zero terminated strings.
    
    // I know I could force it through my own conventions, but this is probably the most platform portable method anyways...
    // :(
    _argument_values_allocation_buffer_allocated += string_size;
    *(_argument_values_allocation_buffer + _argument_values_allocation_buffer_allocated) = 0;
    _argument_values_allocation_buffer_allocated += 1;

    return base_pointer;
}

int blackiron_main(int argc, char** argv);
// NOTE(jerry): temporary globals
static HINSTANCE platform_instance_handle;
// end of temporary globals

// wopengl globals this should be in another file
static PFNWGLCHOOSEPIXELFORMATARBPROC    wglChoosePixelFormatARB;
static PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB;
static PFNWGLMAKECONTEXTCURRENTARBPROC   wglMakeContextCurrentARB;
static PFNWGLSWAPINTERVALEXTPROC         wglSwapIntervalEXT;
// end wopengl globals

// This is some stupid opengl windows crap that you can find from MSDN
void* platform_opengl_load_function(const char* name) {
    void* result = (void*)wglGetProcAddress(name); 

    if (result == NULL       ||
        result == (void*)0x1 ||
        result == (void*)0x2 ||
        result == (void*)0x3 ||
        result == (void*)-1) {
        static HMODULE opengl32_dll = NULL;

        if (opengl32_dll == NULL) {
            opengl32_dll = LoadLibraryA("opengl32.dll");
        }

        return GetProcAddress(opengl32_dll, name);
    }

    return result;
}

int WinMain(HINSTANCE previous_instance, HINSTANCE instance, LPSTR command_line, int show_command) {
    _intentionally_unused(previous_instance);
    _intentionally_unused(show_command);

    {
        platform_instance_handle = instance;
    }

    // I will allocate both of these as static buffers for now
    // since I don't really feel like writing the logic to dynamically allocate these
    // This only takes up (4096) + (1024) = 5120 bytes of memory, which is negligable
    // at the moment. If we seriously need more, or find it different. I will consider
    // dynamically allocating...

    // Although seriously that might spell some seriously concerning problems if that were the
    // case since this is supposed to be for games...
    static size_t _argument_values_allocated = 0;
    static char*  _argument_values[MAX_ARGUMENT_VALUE_STRINGS]      = {0};

    // Build a POSIX compliant command line value string list.
    {
        char module_file_name[255];
        size_t module_file_name_size = GetModuleFileNameA(instance, module_file_name, 255);

        _argument_values[_argument_values_allocated++] = argument_values_add_string(module_file_name, module_file_name_size);

        // I assume that the command_line string is zero terminated.
        char* command_line_string_cursor = command_line;
        while (*command_line_string_cursor) {
            if (is_whitespace_character(*command_line_string_cursor)) {
                continue;
            } else {
                size_t string_begin = (command_line_string_cursor - command_line);

                // eat until next whitespace character
                do {
                    command_line_string_cursor++;
                } while (*command_line_string_cursor &&
                         !is_whitespace_character(*command_line_string_cursor));

                size_t string_end = (command_line_string_cursor - command_line);
                _argument_values[_argument_values_allocated++] = argument_values_add_string(command_line + string_begin, (string_end - string_begin));
            }
            command_line_string_cursor++;
        }
    }

    int return_code = blackiron_main(_argument_values_allocated, _argument_values);
    return return_code;
}

// NOTE(jerry): Please note, this code is nearly identical to Wanderer's platform layer.
// with slight changes.
LRESULT CALLBACK _blackiron_window_procedure_win32(HWND window_handle, UINT message_id, WPARAM parameter0, LPARAM parameter1) {
    LRESULT return_code = 0;

    switch (message_id) {
        case WM_ACTIVATEAPP: {
            // wParam is true if the window is being activated
            // false otherwise.

            // This prevents XInput from continuing polling state
            // unnecessarily (it'll return neutral data.)
            XInputEnable(parameter0);
            input_reset_all_input_state(&application_input);
        } break;

        case WM_PAINT:
        case WM_SIZE: {
            // handle resize? fun
            RECT client_rectangle;
            GetClientRect(window_handle, &client_rectangle);

            int32_t new_screen_width = client_rectangle.right - client_rectangle.left;
            int32_t new_screen_height = client_rectangle.bottom - client_rectangle.top;

            graphics_context_update_screen_dimensions(&application_graphics_context, new_screen_width, new_screen_height);
        } break;

        case WM_CLOSE:
        case WM_DESTROY: {
            application_quit = true;
        } break;

            // text input event
        case WM_CHAR: {
            // TODO(jerry):
            // introduce more general text input.
            // This is literally only used for the console at the moment.
            platform_send_text_input(parameter0);
        } break;

            // mouse events
        case WM_MOUSEMOVE:
        case WM_MBUTTONUP:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_RBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_LBUTTONDOWN: {
            POINT cursor_point;
            GetCursorPos(&cursor_point);
            ScreenToClient(window_handle, &cursor_point);

            // we do not handle relative mode yet.
            // that's just finding the current last known position of the mouse,
            // and snapping it there. Then finding out how much it moves.
            platform_send_mouse_inputs(cursor_point.x, cursor_point.y, (parameter0 & MK_LBUTTON), (parameter0 & MK_MBUTTON), (parameter0 & MK_RBUTTON));
        } break;

            // key events
        /* case WM_SYSCOMMAND: */
        case WM_SYSKEYUP:
        case WM_SYSKEYDOWN:
        case WM_KEYDOWN:
        case WM_KEYUP: {
            uint8_t scancode         = (parameter1 >> 16) & 255;
            uint8_t is_extended      = (parameter1 >> 24) & 1; // I do not really need to handle this, but just incase I suppose.
            int32_t virtual_key_code = parameter0;

            int32_t remapped_key_code_from_virtual_keycode = input_map_native_keycode_to_input_keycode(virtual_key_code);
            int32_t remapped_key_code_from_scancode        = input_map_native_scancode_to_input_keycode(scancode);
            bool    is_key_down                            = (message_id == WM_KEYDOWN) || (message_id == WM_SYSKEYDOWN);

            platform_send_key_inputs(is_key_down, remapped_key_code_from_scancode, remapped_key_code_from_virtual_keycode);
        } break;
        default: {
            return_code = DefWindowProc(window_handle, message_id, parameter0, parameter1);
        } break;
    }

    return return_code;
}

int blackiron_main(int argument_count, char** argument_values) {
    strncpy(_game_dll_name, BLACKIRON_DEFAULT_GAME_DLL_NAME, 255);
    _handle_command_line_arguments(argument_count, argument_values);
    struct host_state host_state = {};
    // Hmmm... Most think a bit carefully of how to do game architecture...
    // let's try some shit I guess! I can use SDL2 as my landing pad at least for
    // platforms I know nothing about!
    RegisterClass(
        &(WNDCLASS) {
            // Hardware acceleration requires the CS_OWNDC per window.
            // I mean it probably shouldn't matter if I make one window, but this is just to be safe.
            .style       = (CS_OWNDC | CS_HREDRAW | CS_VREDRAW),
            .lpfnWndProc = _blackiron_window_procedure_win32,

            // NOTE(jerry): obviously I need to separate this from the windows platform layer
            // for now I will pass hInstance as a global variable.
            .hInstance     = platform_instance_handle,
            .hIcon         = NULL,
            .hCursor       = LoadCursor(NULL, IDC_ARROW),
            .hbrBackground = NULL,
            .lpszMenuName  = NULL,
            .lpszClassName = "blackiron_monkey_class_name"
        }
    );

    // TODO(jerry): allow game to configure window settings and such!

    // NOTE(jerry):
    // little dummy thing to do, since I expect my window client area to
    // actually be of this size.
    // But windows counts the provided width and height as TOTAL area...
    // I presume other OSes might do this too... Anyways yeah.
    const uint32_t window_bitflags = WS_OVERLAPPEDWINDOW ^ (WS_THICKFRAME ^ WS_MAXIMIZEBOX);
    RECT desired_client_area_rectangle = (RECT) {
        .left = 0,
        .top = 0,

        .right = 1280,
        .bottom = 720,
    };

    AdjustWindowRect(&desired_client_area_rectangle, window_bitflags, false);

    int client_width = desired_client_area_rectangle.right - desired_client_area_rectangle.left;
    int client_height = desired_client_area_rectangle.bottom - desired_client_area_rectangle.top;

    global_window_handle = CreateWindowEx(
        0,
        "blackiron_monkey_class_name",
        "BlackIron Monkey",
        // bit flags to disable resizable windows.
        window_bitflags,

        CW_USEDEFAULT, CW_USEDEFAULT,

        client_width, client_height,

        NULL, NULL, platform_instance_handle, NULL
    );

    // setting up opengl context. A more modern one (although I'm probably going to just limit to 2.0 or something)
    // For whatever reason opengl is severely fucked on windows, so the initialization code is hairy since windows only supports
    // opengl 1.1 by default, and gives you the ability to load extensions.
    // Modern OpenGL is considered an extension, so you have to load OpenGL... To load the rest of OpenGL... Cause reasons.
    HDC window_display_context = GetDC(global_window_handle);
    {
        PIXELFORMATDESCRIPTOR pixel_format_descriptor = (PIXELFORMATDESCRIPTOR) {
            .nSize      = sizeof(pixel_format_descriptor),
            .nVersion   = 1,
            .dwFlags    = PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
            .iPixelType = PFD_TYPE_RGBA,

            .cColorBits   = 32,
            .cDepthBits   = 24,
            .cStencilBits = 8,

            .iLayerType = PFD_MAIN_PLANE,
        };

        int pixel_format_index = ChoosePixelFormat(window_display_context, &pixel_format_descriptor);
        SetPixelFormat(window_display_context, pixel_format_index, &pixel_format_descriptor);

        // why does MSDN say the only parameter is unnamedParam1?
        HGLRC dummy_opengl_context = wglCreateContext(window_display_context);
        wglMakeCurrent(window_display_context, dummy_opengl_context);

        {
            wglChoosePixelFormatARB    = platform_opengl_load_function("wglChoosePixelFormatARB");
            wglCreateContextAttribsARB = platform_opengl_load_function("wglCreateContextAttribsARB");
            wglMakeContextCurrentARB   = platform_opengl_load_function("wglMakeContextCurrentARB");
            wglSwapIntervalEXT         = platform_opengl_load_function("wglSwapIntervalEXT");
        }

        // okay, now let's load the real opengl context
        HGLRC opengl_context;
        {
            int pixel_format_attributes_integer_list[] = {
                WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
                WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
                WGL_DOUBLE_BUFFER_ARB,  GL_TRUE,
                WGL_PIXEL_TYPE_ARB,     WGL_TYPE_RGBA_ARB,

                WGL_COLOR_BITS_ARB,   32,
                WGL_DEPTH_BITS_ARB,   24,
                WGL_STENCIL_BITS_ARB, 8,

                0
            };

            int pixel_format;
            unsigned pixel_format_candidate_count;

            wglChoosePixelFormatARB(window_display_context, pixel_format_attributes_integer_list, NULL, 1, &pixel_format, &pixel_format_candidate_count);

            int context_attributes_integer_list[] = {
                WGL_CONTEXT_MAJOR_VERSION_ARB, BLACKIRON_OPENGL_MAJOR_VERSION,
                WGL_CONTEXT_MINOR_VERSION_ARB, BLACKIRON_OPENGL_MINOR_VERSION,
                WGL_CONTEXT_PROFILE_MASK_ARB,
#ifdef BLACKIRON_OPENGL_COMPATIBILITY
                WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
#else
                WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
#endif
                0
            };

            opengl_context = wglCreateContextAttribsARB(window_display_context, NULL, context_attributes_integer_list);
            wglMakeCurrent(window_display_context, opengl_context);

            platform_load_opengl_function_pointers();

            wglDeleteContext(dummy_opengl_context);

            // no vsync
            wglSwapIntervalEXT(0);
        }
    }

    struct host_application_description application = {};
    platform_initialize(&application, &host_state, argument_count, argument_values);

    ShowWindow(global_window_handle, SW_SHOWNORMAL);

    // This is the implicitly linked version that's in the engine.

    // Do this always once. Then we do some dimensional analysis to
    // get the elapsed frame time.
    LARGE_INTEGER frequency_counter;
    QueryPerformanceFrequency(&frequency_counter);

    LARGE_INTEGER start_time;
    LARGE_INTEGER end_time;

    QueryPerformanceCounter(&start_time);

    float last_delta_time = (1/60.0f);

    while (!application_quit) {
        // reset per frame allocator, so we can use it without suspecting anything going wrong!
        // Ever!
        _global_temporary_allocator.used = 0;
        // poll and update controller state
        if (application_poll_controller_state_timer <= 0.0) {
            // Do keep in mind how this might work with SDL2
            // as that's my lingua franca "platform"
            input_poll_for_new_controllers(&application_input);
            application_poll_controller_state_timer = APPLICATION_CONTROLLER_POLL_TIME;
        } else {
            application_poll_controller_state_timer -= last_delta_time;
        }

        input_update_controller_states(&application_input);

        {
            MSG message;

            GetMessage(&message, NULL, 0, 0);
            TranslateMessage(&message);

            // NOTE(jerry):
            // This implicitly handles all the input events we have
            // so remember that!
            DispatchMessage(&message);
        }

        platform_frame(&application, &host_state, last_delta_time);

        wglSwapLayerBuffers(window_display_context, WGL_SWAP_MAIN_PLANE);

        QueryPerformanceCounter(&end_time);
        LARGE_INTEGER elapsed_time = {.QuadPart = end_time.QuadPart - start_time.QuadPart};
        last_delta_time = (elapsed_time.QuadPart / (float)frequency_counter.QuadPart);
        start_time = end_time;
    }

    platform_deinitialize(&application, &host_state);
    return 0;
}
