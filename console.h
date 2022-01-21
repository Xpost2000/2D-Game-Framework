#ifndef CONSOLE_H
#define CONSOLE_H

// This is the shared code for the display and subsystem/variable part

// This is a Quake style console. Everyone loves these!
// There is only one global console.

#include "public/console_api.h"

void console_initialize(struct graphics_context_api* graphics);
void console_printf(char* text, ...);
void console_clear(void);

// ASCII only console.
void console_send_character(char character);
void console_submit(void);

void console_display(struct graphics_context_api* graphics);
void console_allow_submission(void);

// up to the game to respect this.
bool console_active(void);
void console_scroll_by(float amount);

void console_send_key_event(bool key_down_event, uint32_t keycode, bool alt_state, bool ctrl_state, bool shift_state);

static char* console_system_variable_type_as_string(uint8_t type);
static char* console_system_number_type_as_string(uint8_t type);

// These iterators are identical.
typedef struct console_system_variable_iterator {
    struct console_system_variable* current;
} console_system_variable_iterator;
typedef struct console_system_command_iterator {
    struct console_system_command* current;
} console_system_command_iterator;

console_system_variable_iterator console_system_begin_iterating_variables(void);
bool                             console_system_variable_iterator_finished(console_system_variable_iterator* iterator);
struct console_system_variable*  console_system_variable_iterator_advance(console_system_variable_iterator* iterator);

console_system_command_iterator console_system_begin_iterating_commands(void);
bool                            console_system_command_iterator_finished(console_system_command_iterator* iterator);
struct console_system_command*  console_system_command_iterator_advance(console_system_command_iterator* iterator);

void                            console_system_register_command(struct console_system_command* command);
void                            console_system_register_variable(struct console_system_variable* variable);
struct console_system_variable* console_system_find_variable(char* name);


#endif
