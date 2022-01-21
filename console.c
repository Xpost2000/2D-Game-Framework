// TODO(jerry): Handle scrollback or don't!

// The ring buffer implementation does not print whatever is at the head
// as it is being used for writing purposes. So an extra byte is added.
#define CONSOLE_SCROLLBACK_BUFFER_SIZE (16384 + 1)
#define CONSOLE_INPUT_LINE_BUFFER_SIZE (512)

#define CONSOLE_DEFAULT_FONT_PATH                       "resources/console/LiberationMono-Bold.ttf"
#define CONSOLE_CURSOR_BLINK_TIMER_MAX                  (0.75)
#define CONSOLE_SCREEN_PORTION                          (0.65)
#define CONSOLE_INPUT_HISTORY_MAX_ENTRIES               (8)
#define CONSOLE_DROPSHADOW_X_OFFSET                     (2.3)
#define CONSOLE_DROPSHADOW_Y_OFFSET                     (1)
#define CONSOLE_DEFAULT_FONT_SIZE                       (18)
#define CONSOLE_MAXIMUM_ALLOWED_AUTO_COMPLETION_MATCHES (8)

struct console_color {
    float r;
    float g;
    float b;
    float a;
};
struct console_color console_color(float r, float g, float b, float a) {
    return (struct console_color) {
        .r = r,
        .g = g,
        .b = b,
        .a = a
    };
}
struct console_color console_color_from_encoded(uint32_t rgba) {
    struct console_color result;
    decode_rgba_from_uint32(rgba, (float*)&result);
    return result;
}
uint8_t _console_nibble_from_hexadecimal(char hexadecimal_character) {
    // Normalize to uppercase if it's a letter.
    if (hexadecimal_character >= 'a' && hexadecimal_character <= 'z') {
        hexadecimal_character -= 32;
    }

    switch (hexadecimal_character) {
        case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9': {
            return hexadecimal_character - '0';
        } break;
        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': {
            return (hexadecimal_character - 'A') + 10;
        } break;
    }

    return 0;
}
struct console_color console_color_from_hexadecimal_string(char* hexadecimal_string, size_t hexadecimal_string_length) {
    uint32_t encoded_rgba = 0;

    if (hexadecimal_string_length != 6 && hexadecimal_string_length != 8) {
        return console_color_from_encoded(0);
    }

    if (hexadecimal_string_length >= 6) {
        uint8_t bytes[4] = {
            _console_nibble_from_hexadecimal(hexadecimal_string[0]) << 4 | _console_nibble_from_hexadecimal(hexadecimal_string[1]),
            _console_nibble_from_hexadecimal(hexadecimal_string[2]) << 4 | _console_nibble_from_hexadecimal(hexadecimal_string[3]),
            _console_nibble_from_hexadecimal(hexadecimal_string[4]) << 4 | _console_nibble_from_hexadecimal(hexadecimal_string[5]),
            0xFF,
        };

        if (hexadecimal_string_length == 8) {
            bytes[3] = _console_nibble_from_hexadecimal(hexadecimal_string[6]) << 4 | _console_nibble_from_hexadecimal(hexadecimal_string[7]);
        }

        encoded_rgba = bytes[0] << 24 | bytes[1] << 16 | bytes[2] << 8 | bytes[3];
    } else {
        return console_color_from_encoded(0);
    }

    return console_color_from_encoded(encoded_rgba);
}

enum console_animation_state {
    CONSOLE_ANIMATION_STATE_DEFAULT,
    CONSOLE_ANIMATION_STATE_OPENING,
    CONSOLE_ANIMATION_STATE_CLOSING,
};

struct console_theme {
    struct console_color text_color;
    struct console_color text_shadow_color;

    struct console_color background_color;
    struct console_color input_line_color;
    struct console_color completion_suggestion_color;
    struct console_color currently_selection_completion_suggestion_color;

    struct console_color cursor_color;
};

// How I sorely wish these can be default initialized...
// This is the one time I want a constructor.
static struct console_theme default_console_theme = {};

struct console {
    struct console_theme theme;

    struct allocator*            allocator;
    graphics_context_font_handle font;

    char   scrollback_buffer[CONSOLE_SCROLLBACK_BUFFER_SIZE];
    size_t scrollback_buffer_read_cursor;
    size_t scrollback_buffer_write_cursor;

    // since this should be monospaced this is completely fine to use
    struct graphics_context_text_extents character_metrics;

    float scroll_y;
    
    float width;
    float height;

    uint32_t lines_per_page;

    uint32_t input_line_count;
    uint32_t input_line_write_cursor_location;
    char     input_line[CONSOLE_INPUT_LINE_BUFFER_SIZE];

    int8_t  selected_history_buffer_index; // read index
    int8_t  history_buffer_index;
    uint8_t history_buffer_count;

    struct {
        size_t count;
        char   buffer[CONSOLE_INPUT_LINE_BUFFER_SIZE];
    } input_history_buffers[CONSOLE_INPUT_HISTORY_MAX_ENTRIES];

    bool  shown;
    float slide_timer;

    float cursor_blink_timer;
    bool  cursor_show;

    uint8_t animation_state;

    bool    taking_command_suggestions;
    uint8_t command_completion_best_match_selected_index;
    uint8_t command_completion_best_matches_count;
    // The string storage comes from the commands themselves. So this is safe.
    char*   command_completion_best_matches[CONSOLE_MAXIMUM_ALLOWED_AUTO_COMPLETION_MATCHES];
};

struct console_system {
    uint64_t variable_count;
    struct {
        struct console_system_variable* head;
        struct console_system_variable* tail;
        struct console_system_variable null;
    } variable_list;

    uint64_t command_count;
    struct {
        struct console_system_command* head;
        struct console_system_command* tail;
        struct console_system_command null;
    } command_list;
};

static struct console_system _global_console_system = {};
static struct console        _global_console        = {};

void console_clear(void) {
    memset(_global_console.scrollback_buffer, 0, CONSOLE_SCROLLBACK_BUFFER_SIZE);
}

static inline void _console_update_resolution(struct graphics_context_api* graphics) {
    graphics->set_virtual_resolution(0, 0);
    struct graphics_context_virtual_dimensions virtual_dimensions = graphics->virtual_dimensions();

    _global_console.width          = virtual_dimensions.width;
    _global_console.height         = virtual_dimensions.height * CONSOLE_SCREEN_PORTION;
    _global_console.lines_per_page = _global_console.height / (_global_console.character_metrics.height);
}

#include "engine_default_console_commands.c"

// this file was generated with xxd. Don't touch it.
#include "baked_data/liberation_mono_font_data.c"

void console_initialize(struct graphics_context_api* graphics) {
    // hardcoded path.
    // please make sure this is a monospaced font.
    _global_console.font              = graphics_context_load_font_from_buffer_and_keyed_as(&application_graphics_context, resources_console_LiberationMono_Bold_ttf, CONSOLE_DEFAULT_FONT_SIZE, "resources/console/LiberationMono-Bold.ttf");

    _global_console.character_metrics = graphics->measure_text(_global_console.font, "#", 1.0);
    _global_console.allocator         = &_global_temporary_allocator.interface;

    _global_console_system.variable_list.head = &_global_console_system.variable_list.null;
    _global_console_system.variable_list.tail = &_global_console_system.variable_list.null;

    _global_console_system.command_list.head = &_global_console_system.command_list.null;
    _global_console_system.command_list.tail = &_global_console_system.command_list.null;

    _register_default_commands();

    // default theme
    {
        
        default_console_theme = (struct console_theme) {
            .text_color        = console_color(1.0, 1.0, 1.0, 1.0), // white
            .text_shadow_color = console_color(0.0, 0.0, 0.0, 0.8), // transparent black

            .background_color  = console_color_from_encoded(0x008C77EC), // aqua green
            .input_line_color  = console_color_from_encoded(0x005A44FC), // darker aqua green

            .completion_suggestion_color                     = console_color_from_encoded(0x003822FF), // even darker aqua green
            .currently_selection_completion_suggestion_color = console_color_from_encoded(0xEA3C53FF), // reddish

            .cursor_color = console_color(0.0, 1.0, 0.0, 0.8), // green
        };

        _global_console.theme = default_console_theme;
    }

    _console_update_resolution(graphics);
    /* console_clear(); */
}

void console_printf(char* text, ...) {
    static char _temporary_format_buffer[2048] = {};
    memset(_temporary_format_buffer, 0, 2048);

    size_t written;
    {
        va_list variadic_arguments;
        va_start(variadic_arguments, text);
        written = vsnprintf(_temporary_format_buffer, 2048, text, variadic_arguments);
        va_end(variadic_arguments);
    }
    {
        va_list variadic_arguments;
        va_start(variadic_arguments, text);
        vfprintf(stderr, text, variadic_arguments);
        va_end(variadic_arguments);
    }

    // ring buffer writing
    for (size_t character_index = 0; character_index < written; ++character_index) {
        _global_console.scrollback_buffer[_global_console.scrollback_buffer_write_cursor++] = _temporary_format_buffer[character_index];

        // Ring buffer wrap around writing.
        if (_global_console.scrollback_buffer_write_cursor >= CONSOLE_SCROLLBACK_BUFFER_SIZE) {
            _global_console.scrollback_buffer_write_cursor = 0;
            _global_console.scrollback_buffer_read_cursor  += 1;
        }

        // We have overstepped capacity so we will bump the read cursor forward which will "discard" the last
        // character. (This is basically just a queue.)
        if (_global_console.scrollback_buffer_write_cursor == _global_console.scrollback_buffer_read_cursor) {
            _global_console.scrollback_buffer_read_cursor += 1;
        }

        if (_global_console.scrollback_buffer_read_cursor >= CONSOLE_SCROLLBACK_BUFFER_SIZE) {
            _global_console.scrollback_buffer_read_cursor = 0;
        }
    }
}

// This does not need to change for the ring buffer implementation.
// This will always be fine.
static inline uint32_t _console_count_lines(void) {
    uint32_t counted_lines = 0;
    float glyph_width  = _global_console.character_metrics.width;
    float console_width  = _global_console.width;

    char* scrollback_buffer = _global_console.scrollback_buffer;
    float x_cursor = 0;

    for (size_t character_index = 0; character_index < CONSOLE_SCROLLBACK_BUFFER_SIZE; ++character_index) {
        bool should_line_break = false;

        if (scrollback_buffer[character_index] > 0) {
            x_cursor += glyph_width;

            if (x_cursor >= console_width) {
                should_line_break = true;
            }

            if (scrollback_buffer[character_index] == '\n' || scrollback_buffer[character_index] == '\r') {
                should_line_break = true;
            }

            if (should_line_break) {
                x_cursor = 0;
                counted_lines++;
            }
        }
    }

    return counted_lines;
}

void console_scroll_by(float amount) {
    float glyph_height = _global_console.character_metrics.height;

    uint32_t line_count     = _console_count_lines();
    uint32_t lines_per_page = _global_console.lines_per_page;
    float present_pages  = (float)line_count / lines_per_page;

    float scroll_displacement   = (_global_console.scroll_y + amount);
    float scrolled_pages_height = scroll_displacement + lines_per_page * glyph_height;

    _global_console.scroll_y += amount;
}

//please inline
static void _console_display_codepoint(struct graphics_context_api* graphics, uint32_t codepoint, float* x_cursor, float* y_cursor, float glyph_width, float glyph_height, float console_width, float slide_offset_y) {
    bool should_line_break = false;

    if (codepoint != ' ' && is_whitespace_character(codepoint)) {
        switch (codepoint) {
            case '\r':
            case '\n': {
                should_line_break = true;
            } break;
        }
    } else if (codepoint > 0) {
        graphics->draw_codepoint(_global_console.font, codepoint, *x_cursor+CONSOLE_DROPSHADOW_X_OFFSET, *y_cursor+glyph_height+slide_offset_y+CONSOLE_DROPSHADOW_Y_OFFSET, 1.0, 0.0, 0.0, 0.0, 0.8);
        graphics->draw_codepoint(_global_console.font, codepoint, *x_cursor, *y_cursor+glyph_height+slide_offset_y, 1.0, 1.0, 1.0, 1.0, 1.0);
        *x_cursor += (glyph_width);
    }

    {
        float next_glyph_start = *x_cursor + glyph_width;

        if ((next_glyph_start) >= console_width) {
            should_line_break = true;
        }
    }

    if (should_line_break) {
        *x_cursor = 0;
        *y_cursor += (glyph_height);
    }
}

void console_display(struct graphics_context_api* graphics) {
    _console_update_resolution(graphics);
    // To avoid the complexities of supporting virtual resolution,
    // the console will be drawn in real screen resolutions always.
    // cool sliding visual effects I guess
    float slide_offset_y = 0;
    {
        if (_global_console.animation_state == CONSOLE_ANIMATION_STATE_OPENING) {
            slide_offset_y = linear_interpolate_float(_global_console.height, 0, _global_console.slide_timer);
        } else if (_global_console.animation_state == CONSOLE_ANIMATION_STATE_CLOSING) {
            slide_offset_y = linear_interpolate_float(0, -_global_console.height, _global_console.slide_timer);
        }
    }

    // clamping the console to console height.
    float glyph_height = _global_console.character_metrics.height;
    float glyph_width  = _global_console.character_metrics.width;
    uint32_t line_count = _console_count_lines();

    // Reserve 1 line for the input line.
    uint32_t lines_per_page = _global_console.lines_per_page-1;

    // clamp the console scroller.
    float max_scroll_y;
    {
        {
            if (line_count < lines_per_page) {
                line_count = lines_per_page;
            }

            max_scroll_y = (line_count-lines_per_page) * glyph_height;
        }

        if (_global_console.scroll_y < 0) {
            _global_console.scroll_y = 0;
        } else if (_global_console.scroll_y + lines_per_page * glyph_height > line_count * glyph_height) {
            _global_console.scroll_y = max_scroll_y;
        }
    }

    float console_width  = _global_console.width;
    float console_height = _global_console.height;
    float input_line_y   = _global_console.height - glyph_height * 0.7;

    graphics->begin_drawing((struct camera){.scale_x = 1, .scale_y = 1});
    {
        struct console_color color = _global_console.theme.background_color;
        graphics->draw_untextured_quad(0, slide_offset_y, console_width, console_height, color.r, color.g, color.b, color.a, null_shader);
    }
    {
        struct console_color color = _global_console.theme.input_line_color;
        graphics->draw_untextured_quad(0, input_line_y + slide_offset_y, _global_console.width, glyph_height*1.1, color.r, color.g, color.b, color.a, null_shader);
    }
    graphics->end_drawing();

    graphics->begin_drawing((struct camera){.scale_x = 1, .scale_y = 1});
    {
        float x_cursor = 0;
        float y_cursor = -(max_scroll_y);

        graphics->set_scissor_region(0, 0, console_width, input_line_y);

        // ring buffer code makes this a little nastier to do.
        if (_global_console.scrollback_buffer_write_cursor > _global_console.scrollback_buffer_read_cursor) {
            for (decode_utf8_iterator iterator = decode_utf8_from(_global_console.scrollback_buffer, CONSOLE_SCROLLBACK_BUFFER_SIZE);
                 decode_utf8_iterator_valid(&iterator);
                 decode_utf8_iterator_advance(&iterator)) {
                _console_display_codepoint(graphics, iterator.codepoint, &x_cursor, &y_cursor, glyph_width, glyph_height, console_width, slide_offset_y);
            }
        } else if (_global_console.scrollback_buffer_write_cursor < _global_console.scrollback_buffer_read_cursor) {
            // nasty wrap around handling...

            // first half (tail -> end)
            for (decode_utf8_iterator iterator = decode_utf8_from(_global_console.scrollback_buffer + _global_console.scrollback_buffer_read_cursor, CONSOLE_SCROLLBACK_BUFFER_SIZE - _global_console.scrollback_buffer_read_cursor);
                 decode_utf8_iterator_valid(&iterator);
                 decode_utf8_iterator_advance(&iterator)) {
                _console_display_codepoint(graphics, iterator.codepoint, &x_cursor, &y_cursor, glyph_width, glyph_height, console_width, slide_offset_y);
            }

            // second half (begin -> tail)
            for (decode_utf8_iterator iterator = decode_utf8_from(_global_console.scrollback_buffer, _global_console.scrollback_buffer_write_cursor);
                 decode_utf8_iterator_valid(&iterator);
                 decode_utf8_iterator_advance(&iterator)) {
                _console_display_codepoint(graphics, iterator.codepoint, &x_cursor, &y_cursor, glyph_width, glyph_height, console_width, slide_offset_y);
            }
        }
    }
    graphics->end_drawing();

    graphics->begin_drawing((struct camera){.scale_x = 1, .scale_y = 1});
    {
        {
            struct console_color text_color   = _global_console.theme.text_color;
            struct console_color shadow_color = _global_console.theme.text_shadow_color;

            graphics->draw_text(_global_console.font, _global_console.input_line, CONSOLE_DROPSHADOW_X_OFFSET, input_line_y + slide_offset_y + CONSOLE_DROPSHADOW_Y_OFFSET, 1.0, shadow_color.r, shadow_color.g, shadow_color.b, shadow_color.a);
            graphics->draw_text(_global_console.font, _global_console.input_line, 0, input_line_y + slide_offset_y, 1.0, text_color.r, text_color.g, text_color.b, text_color.a);
        }
        if (_global_console.cursor_show) {
            struct console_color cursor_color = _global_console.theme.cursor_color;
            graphics->draw_untextured_quad(_global_console.input_line_write_cursor_location * glyph_width, input_line_y + slide_offset_y, glyph_width, glyph_height, cursor_color.r, cursor_color.g, cursor_color.b, cursor_color.a, null_shader);
        }
    }
    graphics->end_drawing();

    // NOTE(jerry): Because of the way my graphics_context works... This has to be done in two passes :(
    if (_global_console.command_completion_best_matches_count > 0) {
        {
            {
                float autocompletion_y_cursor = input_line_y + glyph_height*1.1;
                
                graphics->begin_drawing((struct camera){.scale_x = 1, .scale_y = 1});

                struct console_color selected_color = _global_console.theme.currently_selection_completion_suggestion_color;
                struct console_color color          = _global_console.theme.completion_suggestion_color;

                for (size_t command_completion_suggestion_index = 0; command_completion_suggestion_index < _global_console.command_completion_best_matches_count; ++command_completion_suggestion_index) {
                    if (command_completion_suggestion_index == _global_console.command_completion_best_match_selected_index) {
                        graphics->draw_untextured_quad(0, autocompletion_y_cursor + slide_offset_y, _global_console.width, glyph_height*1.1, selected_color.r, selected_color.g, selected_color.b, selected_color.a, null_shader);
                    } else {
                        graphics->draw_untextured_quad(0, autocompletion_y_cursor + slide_offset_y, _global_console.width, glyph_height*1.1, color.r, color.g, color.b, color.a, null_shader);
                    }
                    autocompletion_y_cursor += glyph_height * 1.1;
                }
                graphics->end_drawing();
            }

            {
                float                autocompletion_y_cursor = input_line_y + glyph_height*1.1;
                struct console_color color                   = _global_console.theme.text_color;

                graphics->begin_drawing((struct camera){.scale_x = 1, .scale_y = 1});
                for (size_t command_completion_suggestion_index = 0; command_completion_suggestion_index < _global_console.command_completion_best_matches_count; ++command_completion_suggestion_index) {
                    char* current_suggestion = _global_console.command_completion_best_matches[command_completion_suggestion_index];
                    graphics->draw_text(_global_console.font, current_suggestion, 0, autocompletion_y_cursor + slide_offset_y, 1.0, color.r, color.g, color.b, color.a);
                    autocompletion_y_cursor += glyph_height * 1.1;
                }
                graphics->end_drawing();
            }

        }
    }
}

void console_system_execute(char* line, size_t line_size);
void console_submit(void) {
    if (_global_console.input_line_count > 0) {
        console_system_execute(_global_console.input_line, _global_console.input_line_count);

        memcpy(_global_console.input_history_buffers[_global_console.history_buffer_index].buffer, _global_console.input_line, CONSOLE_INPUT_LINE_BUFFER_SIZE);
        _global_console.input_history_buffers[_global_console.history_buffer_index++].count = _global_console.input_line_count;
        _global_console.history_buffer_index &= (CONSOLE_INPUT_HISTORY_MAX_ENTRIES-1);

        if (_global_console.history_buffer_count < CONSOLE_INPUT_HISTORY_MAX_ENTRIES) {
            _global_console.history_buffer_count++;
        }

        _global_console.input_line_count = 0;
        memset(_global_console.input_line, 0, CONSOLE_INPUT_LINE_BUFFER_SIZE);
    }
}

void console_send_character(char character) {
    if (!console_active()) {
        return;
    }

    // reject characters we use for our own purposes.
    if (character == '`' || character == '~' || character == '\t') {
        return;
    }

    if (_global_console.input_line_count >= CONSOLE_INPUT_LINE_BUFFER_SIZE || character == '\r' || character == '\n') {
        return;
    }

    // This looks a little nasty, but I'm not using a gap-buffer or anything
    // this is just a normal "string" operation, so it's technically slow, but practically fast!
    if (character == '\b') {
        if (_global_console.input_line_count > 0) {
            for (size_t cursor_location = _global_console.input_line_write_cursor_location-1; cursor_location < _global_console.input_line_count; ++cursor_location) {
                _global_console.input_line[cursor_location] = _global_console.input_line[cursor_location+1];
            }
            _global_console.input_line[--_global_console.input_line_count] = 0;
            --_global_console.input_line_write_cursor_location;
        } else if (_global_console.input_line_count == 0) {
            _global_console.input_line[0] = 0;
        }

    } else if (is_human_readable_ascii_character(character)) {
        for (size_t cursor_location = _global_console.input_line_count; cursor_location > _global_console.input_line_write_cursor_location; --cursor_location) {
            _global_console.input_line[cursor_location] = _global_console.input_line[cursor_location-1];
        }

        _global_console.input_line_count++;
        _global_console.input_line[_global_console.input_line_write_cursor_location++] = character;
    }

    _global_console.command_completion_best_matches_count = 0;
    _global_console.taking_command_suggestions            = false;
}

bool console_active(void) {
    return _global_console.shown;
}

void console_move_forward_character(void) {
    if (_global_console.input_line_write_cursor_location+1 <= _global_console.input_line_count) {
        _global_console.input_line_write_cursor_location++;
    }
}

void console_move_backward_character(void) {
    if (_global_console.input_line_write_cursor_location > 0) {
        _global_console.input_line_write_cursor_location--;
    }
}

void console_begin_command_completion(void) {
    if (!_global_console.taking_command_suggestions) {
        _global_console.command_completion_best_matches_count        = 0;
        _global_console.command_completion_best_match_selected_index = 0;
        _global_console.taking_command_suggestions                   = true;

        void _console_determine_best_completion_match_for_partial_command_string(char* command_string);
        _console_determine_best_completion_match_for_partial_command_string(_global_console.input_line);
    }
}

void console_input_command_suggestion(size_t command_suggestion_index) {
    if (command_suggestion_index < _global_console.command_completion_best_matches_count) {
        char*  suggestion_string           = _global_console.command_completion_best_matches[command_suggestion_index];
        size_t length_of_suggestion_string = cstring_length(suggestion_string);

        memset(_global_console.input_line, 0, _global_console.input_line_count);
                        
        _global_console.input_line_count                 = length_of_suggestion_string;
        _global_console.input_line_write_cursor_location = _global_console.input_line_count;
        strncpy(_global_console.input_line, suggestion_string, length_of_suggestion_string);
    }
}

void console_stop_command_completion(void) {
    if (_global_console.taking_command_suggestions) {
        _global_console.command_completion_best_matches_count = 0;
        _global_console.taking_command_suggestions            = false;
    }
}

void console_previous_command_completion_suggestion(void) {
    if (_global_console.taking_command_suggestions) {
        if (_global_console.command_completion_best_match_selected_index > 0) {
            _global_console.command_completion_best_match_selected_index -= 1;
        } else {
            _global_console.command_completion_best_match_selected_index = _global_console.command_completion_best_matches_count-1;
        }
    }
}

void console_next_command_completion_suggestion(void) {
    if (_global_console.taking_command_suggestions) {
        _global_console.command_completion_best_match_selected_index += 1;
        if (_global_console.command_completion_best_match_selected_index >= _global_console.command_completion_best_matches_count) {
            _global_console.command_completion_best_match_selected_index = 0;
        }
    }
}

void console_move_forward_word(void) {
    // First finish the current word, by running into whitespace.
    while (_global_console.input_line_write_cursor_location < _global_console.input_line_count && !is_whitespace_character(_global_console.input_line[_global_console.input_line_write_cursor_location])) {
        console_move_forward_character();
    }

    // Then find the next word by looking for the first next non-whitespace character.
    while (_global_console.input_line_write_cursor_location < _global_console.input_line_count && is_whitespace_character(_global_console.input_line[_global_console.input_line_write_cursor_location])) {
        console_move_forward_character();
    }
}

void console_move_backward_word(void) {
    // same as the above. Just in reverse.
    while (_global_console.input_line_write_cursor_location > 0 && !is_whitespace_character(_global_console.input_line[_global_console.input_line_write_cursor_location])) {
        console_move_backward_character();
    }

    while (_global_console.input_line_write_cursor_location > 0 && is_whitespace_character(_global_console.input_line[_global_console.input_line_write_cursor_location])) {
        console_move_backward_character();
    }
}

void console_kill_line_at_current_position(void) {
    if (_global_console.input_line_write_cursor_location < _global_console.input_line_count) {
        memset(_global_console.input_line + _global_console.input_line_write_cursor_location, 0, _global_console.input_line_count - _global_console.input_line_write_cursor_location);
        _global_console.input_line_count = strlen(_global_console.input_line);
    }
}

void console_previous_history_item(void) {
    if (_global_console.selected_history_buffer_index < 0) {
        _global_console.selected_history_buffer_index = _global_console.history_buffer_count - 1;
        if (_global_console.selected_history_buffer_index < 0) {
            return;
        }
    }
    memcpy(_global_console.input_line, _global_console.input_history_buffers[_global_console.selected_history_buffer_index].buffer, CONSOLE_INPUT_LINE_BUFFER_SIZE);
    _global_console.input_line_write_cursor_location = 0;
    _global_console.input_line_count                 = _global_console.input_history_buffers[_global_console.selected_history_buffer_index--].count;
}

void console_next_history_item() {
    if (_global_console.selected_history_buffer_index >= _global_console.history_buffer_count) {
        _global_console.selected_history_buffer_index = 0;
    }
    memcpy(_global_console.input_line, _global_console.input_history_buffers[_global_console.selected_history_buffer_index].buffer, CONSOLE_INPUT_LINE_BUFFER_SIZE);
    _global_console.input_line_write_cursor_location = 0;
    _global_console.input_line_count                 = _global_console.input_history_buffers[_global_console.selected_history_buffer_index++].count;
}

void console_send_key_event(bool key_down_event, uint32_t keycode, bool alt_state, bool ctrl_state, bool shift_state) {
    if (!console_active()) {
        return;
    }

    if (key_down_event) {
        // emacs / readline bindings
        if (ctrl_state) {
            console_stop_command_completion();
            switch (keycode) {
                case INPUT_KEY_A: {
                    _global_console.input_line_write_cursor_location = 0;
                } break;
                case INPUT_KEY_E: {
                    _global_console.input_line_write_cursor_location = _global_console.input_line_count;
                } break;
                case INPUT_KEY_F: {
                    console_move_forward_character();
                } break;
                case INPUT_KEY_B: {
                    console_move_backward_character();
                } break;
                case INPUT_KEY_K: {
                    console_kill_line_at_current_position();
                } break;
            }
        } else if (alt_state) {
            console_stop_command_completion();
            switch (keycode) {
                case INPUT_KEY_F: {
                    console_move_forward_word();
                } break;
                case INPUT_KEY_B: {
                    console_move_backward_word();
                } break;
            }
        } else {
            switch (keycode) {
                case INPUT_KEY_RIGHT: {
                    console_move_forward_character();
                    console_stop_command_completion();
                } break;
                case INPUT_KEY_LEFT: {
                    console_move_backward_character();
                    console_stop_command_completion();
                } break;

                    // go through the history buffers...
                    // This will "forget" the currently typed line, so be ware.
                case INPUT_KEY_UP: {
                    console_previous_history_item();
                    console_stop_command_completion();
                } break;
                case INPUT_KEY_DOWN: {
                    console_next_history_item();
                    console_stop_command_completion();
                } break;

                case INPUT_KEY_ESCAPE: {
                    console_stop_command_completion();
                } break;

                case INPUT_KEY_RETURN: {
                    if (_global_console.taking_command_suggestions && _global_console.command_completion_best_matches_count > 0) {
                        console_input_command_suggestion(_global_console.command_completion_best_match_selected_index);
                    } else {
                        console_submit();
                    }

                    console_stop_command_completion();
                } break;

                case INPUT_KEY_TAB: {

                    if (_global_console.taking_command_suggestions) {
                        if (shift_state) {
                            console_previous_command_completion_suggestion();
                        } else {
                            console_next_command_completion_suggestion();
                        }
                    } else {
                        if (_global_console.input_line_count > 0) {
                            console_begin_command_completion();
                        } else {
                            console_stop_command_completion();
                        }
                    }
                } break;
            }
        }
    }
}

static void console_frame(struct graphics_context_api* graphics, struct input_api* input, float delta_time) {
    if (input->key_pressed(INPUT_KEY_BACKQUOTE)) {
        // only allow actions when the animation is finished.
        // Or we are not animating.
        if (_global_console.slide_timer == 0.0 || _global_console.animation_state == CONSOLE_ANIMATION_STATE_DEFAULT) {
            if (_global_console.shown == false) {
                _global_console.shown = true;
                _global_console.animation_state = CONSOLE_ANIMATION_STATE_OPENING;
            } else if (_global_console.shown == true) {
                _global_console.shown = false;
                _global_console.animation_state = CONSOLE_ANIMATION_STATE_CLOSING;
            }
            
            _global_console.slide_timer = 0;
        }
    }

    _global_console.cursor_blink_timer += delta_time;
    _global_console.slide_timer        += delta_time * 2.15;

    if (_global_console.input_line_write_cursor_location > _global_console.input_line_count) {
        _global_console.input_line_write_cursor_location = _global_console.input_line_count;
    }

    if (_global_console.cursor_blink_timer >= CONSOLE_CURSOR_BLINK_TIMER_MAX) {
        _global_console.cursor_blink_timer = 0;
        _global_console.cursor_show       ^= true;
    }

    if (_global_console.slide_timer >= 1.0) {
        _global_console.slide_timer = 1.0;
        _global_console.animation_state = CONSOLE_ANIMATION_STATE_DEFAULT;
    }

    // When we are no longer animating,
    // and we are not trying to show ourselves, we don't do anything else.
    if (!_global_console.shown && _global_console.animation_state == CONSOLE_ANIMATION_STATE_DEFAULT) {
        return;
    }

    console_display(graphics);

#if 0
    // not doing scrollback yet.
    {
        const float CONSOLE_SCROLL_SPEED = 300;
        if (input->key_down(INPUT_KEY_UP)) {
            console_scroll_by(-CONSOLE_SCROLL_SPEED * last_delta_time);
        } else if (input->key_down(INPUT_KEY_DOWN)) {
            console_scroll_by(CONSOLE_SCROLL_SPEED * last_delta_time);
        }
    }
#endif
}

static char* console_system_variable_type_as_string(uint8_t type) {
    switch (type) {
        case CONSOLE_VARIABLE_TYPE_NUMBER:  return "number";
        case CONSOLE_VARIABLE_TYPE_STRING:  return "string";
        case CONSOLE_VARIABLE_TYPE_BOOLEAN: return "boolean";
    } 
    return "unknown";
}
static char* console_system_number_type_as_string(uint8_t type) {
    switch (type) {
        case CONSOLE_VARIABLE_TYPE_NUMBER_REAL:    return "real";
        case CONSOLE_VARIABLE_TYPE_NUMBER_INTEGER: return "integer";
    }
    return "unknown";
}

console_system_variable_iterator console_system_begin_iterating_variables(void) {
    return (console_system_variable_iterator) {
        .current = _global_console_system.variable_list.head
    };
}
bool console_system_variable_iterator_finished(console_system_variable_iterator* iterator) {
    if (iterator->current == &_global_console_system.variable_list.null) {
        return true;
    }

    return false;
}
struct console_system_variable* console_system_variable_iterator_advance(console_system_variable_iterator* iterator) {
    struct console_system_variable* current = iterator->current;
    iterator->current = current->next;
    return current;
}

// duped.
console_system_command_iterator console_system_begin_iterating_commands(void) {
    return (console_system_command_iterator) {
        .current = _global_console_system.command_list.head
    };
}
bool console_system_command_iterator_finished(console_system_command_iterator* iterator) {
    if (iterator->current == &_global_console_system.command_list.null) {
        return true;
    }

    return false;
}
struct console_system_command* console_system_command_iterator_advance(console_system_command_iterator* iterator) {
    struct console_system_command* current = iterator->current;
    iterator->current = current->next;
    return current;
}

size_t console_system_command_count(void) {
    size_t length = 0;
    for (console_system_command_iterator iterator = console_system_begin_iterating_commands();
         !console_system_command_iterator_finished(&iterator);
         console_system_command_iterator_advance(&iterator), length++) {
    }
    return length;
}

size_t console_system_variable_count(void) {
    size_t length = 0;
    for (console_system_variable_iterator iterator = console_system_begin_iterating_variables();
         !console_system_variable_iterator_finished(&iterator);
         console_system_variable_iterator_advance(&iterator), length++) {
    }
    return length;
}

void console_system_register_command(struct console_system_command* command) {
    struct console_system_command* sentinel = &_global_console_system.command_list.null;
    _global_console_system.command_count++;

    if (_global_console_system.command_list.head == sentinel) {
        _global_console_system.command_list.head = command;
    } else {
        struct console_system_command* old_tail = _global_console_system.command_list.tail;
        old_tail->next = command;
    }

    _global_console_system.command_list.tail = command;
    command->next = sentinel; 
}

void console_system_register_variable(struct console_system_variable* variable) {
    struct console_system_variable* sentinel = &_global_console_system.variable_list.null;
    _global_console_system.variable_count++;

    if (_global_console_system.variable_list.head == sentinel) {
        _global_console_system.variable_list.head = variable;
    } else {
        struct console_system_variable* old_tail = _global_console_system.variable_list.tail;
        old_tail->next = variable;
    }

    _global_console_system.variable_list.tail = variable;
    variable->next = sentinel; 
}
    
struct console_system_variable* console_system_find_variable(char* name) {
    for (console_system_variable_iterator iterator = console_system_begin_iterating_variables();
         !console_system_variable_iterator_finished(&iterator);
         console_system_variable_iterator_advance(&iterator)) {
        struct console_system_variable* current = iterator.current;

        if (strcmp(current->name, name) == 0) {
            return current;
        }
    }

    return &_global_console_system.variable_list.null;
}

// NOTE(jerry): this isn't exposed because you shouldn't have to read functions...
// TODO(jerry): differs from find_variable which uses a C string. This is because we use the tokenizer which
// provides a length encoded string! I need to make up my mind on whether to use C strings or go length encoded!
struct console_system_command* console_system_find_command(char* name, size_t name_length) {
    for (console_system_command_iterator iterator = console_system_begin_iterating_commands();
         !console_system_command_iterator_finished(&iterator);
         console_system_command_iterator_advance(&iterator)) {
        struct console_system_command* current = iterator.current;

        size_t length_of_name = strlen(current->name);
        if (name_length == length_of_name) {
            if (strncmp(current->name, name, name_length) == 0) {
                return current;
            }
        }
    }

    return &_global_console_system.command_list.null;
}

static void _console_system_call_procedure_and_do_typechecking(struct console_system_command* command, uint8_t parameter_count, struct console_system_variant* parameters) {
    bool allow_procedure_call = true;

    if (allow_procedure_call) {
        command->procedure(parameter_count, parameters);
    }
}

// Some kind of arbitrary string scoring system.
// This has no normalized or determined range.
// However it does differentiate correctly between the matchness of a string.
// NOTE(jerry): Keep fine tuning this.
int32_t _console_determine_string_match_score_of(char* master_string, char* to_match_string) {
    int32_t string_match_score = 0;

    size_t master_string_length   = cstring_length(master_string);
    size_t to_match_string_length = cstring_length(to_match_string);

    if (to_match_string_length == master_string_length) {
        string_match_score += 25;
    } else {
        string_match_score -= 25;
    }

    size_t shorter_string_length = to_match_string_length;
    if (master_string_length < to_match_string_length) {
        shorter_string_length = master_string_length;
    }

    for (size_t first_character_index = 0; first_character_index < shorter_string_length; ++first_character_index) {
        char character_to_match = master_string[first_character_index];

        if (to_match_string[first_character_index] == character_to_match) {
            string_match_score += 42;
        } else {
            string_match_score -= 35;

            // NOTE(jerry):
            // this isn't perfectly accurate.
            // As this will not trigger on the correct instance of the letter.
            // but it'll probably look good enough.
            for (size_t second_character_index = 0; second_character_index < to_match_string_length; ++second_character_index) {
                if (to_match_string[second_character_index] == character_to_match) {
                    string_match_score += 32;
                } else {
                    string_match_score -= 1;
                }
            }
        }
    }

    return string_match_score;
}

// Call this when pressing tab for tab completion.
// This will update to command_completion_best_matches list that can be used
// later.
void _console_determine_best_completion_match_for_partial_command_string(char* command_string) {
    struct console_system_command* existing_command = console_system_find_command(command_string, strlen(command_string));
    _global_console.command_completion_best_matches_count = 0;

    if (existing_command != &_global_console_system.command_list.null) {
        // the partial string is actually just a command.
        _global_console.command_completion_best_matches_count = 1;
        _global_console.command_completion_best_matches[0]    = existing_command->name;
    } else {
        struct _console_command_completion_score_pair {
            char*   command_name;
            int32_t score;
        };

        size_t                                         score_count = console_system_command_count();
        struct _console_command_completion_score_pair* scores      = _global_console.allocator->allocate(_global_console.allocator, sizeof(*scores) * score_count);

        int32_t highest_encountered_score = -INT_MAX;
        // Map all the commands into a name, score pair.
        {
            size_t score_index = 0;
            for (console_system_command_iterator iterator = console_system_begin_iterating_commands();
                 !console_system_command_iterator_finished(&iterator);
                 console_system_command_iterator_advance(&iterator), score_index++) {
                struct console_system_command* current = iterator.current;

                scores[score_index] = (struct _console_command_completion_score_pair) {
                    .command_name = current->name,
                    .score        = _console_determine_string_match_score_of(command_string, current->name),
                };

                if (scores[score_index].score > highest_encountered_score) {
                    highest_encountered_score = scores[score_index].score;
                }
            }
        }

        // basic insertion sort on all commands. To order them
        {
            for (size_t first_index = 1; first_index < score_count; ++first_index) {
                struct _console_command_completion_score_pair score_pair_to_insert = scores[first_index];

                size_t /*insertion_index*/ second_index = first_index;
                for (; second_index > 0; --second_index) {
                    struct _console_command_completion_score_pair score_pair_to_compare_against = scores[second_index-1];

                    if (score_pair_to_compare_against.score > score_pair_to_insert.score) {
                        break;
                    } else {
                        scores[second_index] = scores[second_index - 1];
                    }
                }

                scores[/*insertion_index*/ second_index] = score_pair_to_insert;
            }
        }

        // Insert the first 8 based on the highest score within a certain tolerance range.
        // which should be fine tuned...
        {
            const float tolerance_percentage = 0.37654321;
            int32_t upper_tolerance_score    = (int32_t) (highest_encountered_score * (1.0 + tolerance_percentage) + 0.5);
            int32_t lower_tolerance_score    = (int32_t) (highest_encountered_score * (1.0 - tolerance_percentage) + 0.5);

            for (size_t score_index = 0; score_index < score_count && _global_console.command_completion_best_matches_count < CONSOLE_MAXIMUM_ALLOWED_AUTO_COMPLETION_MATCHES; ++score_index) {
                if (scores[score_index].score >= lower_tolerance_score && scores[score_index].score <= upper_tolerance_score) {
                    _global_console.command_completion_best_matches[_global_console.command_completion_best_matches_count++] = scores[score_index].command_name;
                }
            }
        }

        _global_console.allocator->free(_global_console.allocator, scores);
    }
}

// Technically, just like Quake. This can become a state manager in the most disturbing way.
// Just a slower one because it's a linked list (and these are not pooled.)
void console_system_execute(char* line, size_t line_size) {
    struct console_system_command* command = &_global_console_system.command_list.null;
    word_iterator                  words   = word_iterator_begin_iterating_from(line, line_size);

    uint8_t                       parameter_count                                   = 0;
    struct console_system_variant parameters[CONSOLE_SYSTEM_COMMANDS_MAX_ARGUMENTS] = {};

    if (word_iterator_length(&words)) {
        command = console_system_find_command(words.word, words.word_length);

        // bad command. very very bad.
        if (command == &_global_console_system.command_list.null) {
            console_printf("No command by the name of \"%.*s\"\n", words.word_length, words.word);
            return;
        }

        word_iterator_advance(&words);
        while (!word_iterator_finished(&words) && parameter_count < CONSOLE_SYSTEM_COMMANDS_MAX_ARGUMENTS) {
            struct console_system_variant* current_parameter = &parameters[parameter_count++];

            // inner string validation to check for the type I want,
            // this kind of thing has appeared several times already in minischeme.c. Just ignore it.
            uint8_t type = CONSOLE_VARIABLE_TYPE_STRING;
            {
                bool numeric     = true;
                bool real_number = false;

                for (size_t character_index = 0; character_index < words.word_length; ++character_index) {
                    // should really be doing more rigorous input validation.
                    if (!is_numeric_character(words.word[character_index]) && words.word[character_index] != '.') {
                        numeric = false;
                    } else if (words.word[character_index] == '.' && numeric == true) {
                        real_number = true;
                    }
                }

                if (numeric) {
                    type = CONSOLE_VARIABLE_TYPE_NUMBER;

                    if (real_number) {
                        type |= CONSOLE_VARIABLE_TYPE_NUMBER_REAL;
                    } else {
                        type |= CONSOLE_VARIABLE_TYPE_NUMBER_INTEGER;
                    }
                }
            }

            if (type & CONSOLE_VARIABLE_TYPE_NUMBER) {
                // I'm assuming that atof and atoi actually stop at the end of a valid number
                if (type & CONSOLE_VARIABLE_TYPE_NUMBER_REAL) {
                    float value = atof(words.word);
                    current_parameter->real = value;
                } else if (type & CONSOLE_VARIABLE_TYPE_NUMBER_INTEGER) {
                    int value = atoi(words.word);
                    current_parameter->integer = value;
                }
            } else {
                switch (type) {
                    case CONSOLE_VARIABLE_TYPE_STRING: {
                        // not case insensitive.
                        // whoops.
                        bool is_true  = strncmp(words.word, "true", CONSOLE_SYSTEM_VARIABLE_STRING_SIZE) == 0;
                        bool is_false = strncmp(words.word, "false", CONSOLE_SYSTEM_VARIABLE_STRING_SIZE) == 0;

                        if (is_true || is_false) {
                            type = CONSOLE_VARIABLE_TYPE_BOOLEAN;

                            bool value;
                            // LOL, pretty sure this doesn't need to be like that but hahaha.
                            if (is_false) {
                                value = false;                            
                            } else {
                                value = true;
                            }

                            current_parameter->boolean = value;
                        } else {
                            size_t copy_length = words.word_length;
                            if (copy_length > CONSOLE_SYSTEM_VARIABLE_STRING_SIZE) {
                                copy_length = CONSOLE_SYSTEM_VARIABLE_STRING_SIZE;
                            }

                            strncpy(current_parameter->string, words.word, words.word_length);
                        }
                    } break;
                }
            }

            current_parameter->type = type;
            word_iterator_advance(&words);
        }
        
        _console_system_call_procedure_and_do_typechecking(command, parameter_count, parameters);
    } else {
        return;
    }
}
