#ifndef CONSOLE_API_H
#define CONSOLE_API_H

#define CONSOLE_SYSTEM_VARIABLE_STRING_SIZE (64)
#define CONSOLE_SYSTEM_COMMANDS_MAX_ARGUMENTS (32)

#include "macro_helpers.h"

// Here are some macros to make life more pleasant,
// you still have to register them manually though.
// It will produce variables as cmd_NAME
#define Define_Console_Command(Name)                                                                                                         \
    void template_fn_macro_concatenate(_console_command_generated, Name)(uint8_t argument_count, struct console_system_variant* parameters); \
    static struct console_system_command template_fn_macro_concatenate(cmd, Name) = {                                                        \
        .name = stringify(Name),                                                                                                             \
        .procedure = template_fn_macro_concatenate(_console_command_generated, Name)                                                         \
    };                                                                                                                                       \
    void template_fn_macro_concatenate(_console_command_generated, Name)(uint8_t argument_count, struct console_system_variant* parameters)

// initialize elsewhere because of the way strings work.
// As these strings are writable, and not read-only.
#define Define_Console_Variable(Name, Type)                                               \
    static struct console_system_variable template_fn_macro_concatenate(convar, Name) = { \
        .name = stringify(Name),                                                          \
        .type = template_fn_macro_concatenate(CONSOLE_VARIABLE_TYPE, Type)                \
    }

enum console_system_variable_type {
    CONSOLE_VARIABLE_TYPE_NUMBER  = BIT(0),
    CONSOLE_VARIABLE_TYPE_STRING  = BIT(1),
    CONSOLE_VARIABLE_TYPE_BOOLEAN = BIT(2),
    CONSOLE_VARIABLE_TYPE_COUNT,
};
enum console_system_number_type {
    // This is setup like this so you can just do
    /*
      console_system_variable sv_cheats = { 
            .name = "sv_cheats",
            .type = CONSOLE_VARIABLE_TYPE_NUMBER | CONSOLE_VARIABLE_TYPE_NUMBER_INTEGER,
            .integer = 0
      };
     */
    CONSOLE_VARIABLE_TYPE_NUMBER_REAL    = BIT(1),
    CONSOLE_VARIABLE_TYPE_NUMBER_INTEGER = BIT(2),
    CONSOLE_VARIABLE_TYPE_NUMBER_COUNT,
};

struct console_system_variant {
    uint8_t type;
    union {
        int32_t integer;
        float   real;
        char    string[CONSOLE_SYSTEM_VARIABLE_STRING_SIZE];
        bool    boolean;
    };
};
struct console_system_variable {
    char*   name;
    uint8_t type;
    union {
        int32_t integer;
        float   real;
        char    string[CONSOLE_SYSTEM_VARIABLE_STRING_SIZE];
        bool    boolean;
    };

    struct console_system_variable* next;
};

typedef void (*console_system_command_procedure)(uint8_t argument_count, struct console_system_variant* parameters);

// these are "type-checked."
struct console_system_command {
    char* name;
    console_system_command_procedure procedure;
    struct console_system_command* next;
};

struct console_api {
    void   (*register_variable)(struct console_system_variable*);
    void   (*register_command)(struct console_system_command*);

    struct console_system_variable* (*find_variable)(char*);

    bool (*active)(void);
    void (*printf)(char*, ...);
    void (*clear)(void);
};

#endif
