#ifndef MINI_SCHEME_H
#define MINI_SCHEME_H

// NOTE(jerry):
// This can get a bit piggy if I'm not careful on memory.

// Mini Scheme is not actually an implementation of scheme, but it's just a miniture lisp like language
// this is primarily used for data.
// It may become an interpreted language or something

// since it's easy to write a parser for

// This is one of the few things that isn't managed by handles. Instead it's managed
// by raw pointers.

// Also this is one of the files that uses lots of dynamic allocation, which I'm okay with here
// since I'm only using it to read data or very light scripting if it really comes to that.

// I would've provided a different type of allocator, however the issue happens when I have to do
// eval, which would require a GC of some kind, which I'd probably just do reference counting for
// and hope nothing explodes.

// I may add an object pool soon enough, but if I'm keeping this as a data format... This should
// be totally fine.

// Honestly if I really did need a scripting language, it's likely I'd just hoist on Lua again...
enum minischeme_form_type {
    MINISCHEME_FORM_NIL,     // NOTE(jerry): SYMBOL NOT GENERATED YET
    MINISCHEME_FORM_SYMBOL,
    MINISCHEME_FORM_STRING,
    MINISCHEME_FORM_NUMBER,
    MINISCHEME_FORM_LIST,
    MINISCHEME_FORM_TRUE,   // SYMBOL NOT GENERATED YET
    MINISCHEME_FORM_FALSE,  // SYMBOL NOT GENERATED YET
};
static char* minischeme_form_type_strings[] = {
    "nil",
    "symbol",
    "string",
    "number",
    "list",
    "true",
    "false"
};

#define MINISCHEME_FORM_STRING_MAX_LENGTH (128)
// symbols are hashed into a table
struct minischeme_form_symbol {
    uint32_t index;
};
struct minischeme_form_string {
    uint32_t length;
    char string[MINISCHEME_FORM_STRING_MAX_LENGTH];
};
enum minischeme_form_number_type {
    MINISCHEME_FORM_NUMBER_INTEGER,
    MINISCHEME_FORM_NUMBER_REAL,
};
struct minischeme_form_number {
    uint8_t number_type;
    union {
        float   real;
        int32_t integer;
    };
};
struct minischeme_form {
    uint8_t type;
    union {
        struct minischeme_form_string string;
        struct minischeme_form_number number;
        struct minischeme_form_symbol symbol;
        struct minischeme_form_list*   list;
    };
};

#define type_tag struct
#define type minischeme_form
#include "dynamic_array.h"

// NOTE(jerry):
// This is just to be pedantic. I should really just make an array.
// Since it'd be more performant that way...

// cons-cell.
struct minischeme_form_list {
    struct minischeme_form car;
    struct minischeme_form_list* cdr;
};
size_t minischeme_form_list_length(struct minischeme_form* list);
struct minischeme_form* minischeme_form_list_elt(struct minischeme_form* list, size_t index);
struct minischeme_form* minischeme_form_list_last(struct minischeme_form* list);
struct minischeme_form* minischeme_form_list_first(struct minischeme_form* list);

// an allocator
// forms, and a symbol table probably.
#define MINISCHEME_SYMBOL_TABLE_LENGTH (16384)

// careful with the symbol count.
// TODO(jerry): take allocator!
struct minischeme_toplevel {
    // hash table
    struct minischeme_form_string*        symbol_table;
    struct dynamic_array(minischeme_form) forms;
};

// TODO(jerry):
// make this exposed to an API.
// So it can be used from client code safely without requiring the original source.

struct minischeme_toplevel minischeme_toplevel_load_from_string(char* string, size_t length);
struct minischeme_toplevel minischeme_toplevel_load_from_cstring(char* string);
struct minischeme_toplevel minischeme_toplevel_load_from_file(struct system_api* system, char* file_location);
void                       minischeme_toplevel_free(struct minischeme_toplevel* instance);

struct minischeme_form_string minischeme_toplevel_symbol_name_from_form(struct minischeme_toplevel* toplevel, struct minischeme_form form);

/*
  If I were to ever do eval
  
  environment is something else.
  
  struct minischeme_form minischeme_eval_form(struct minischeme_environment* environment, struct minischeme_form* form);
  struct minischeme_form minischeme_eval_form_from_cstring(struct minischeme_environment* environment, char* string);
  struct minischeme_form minischeme_eval_form_from_string(struct minischeme_environment* environment, char* string, size_t string_length);  
*/

// Some of the API access stuff... Is kind of nasty, and I'm not a huge fan of this right now
// mostly cause I honestly haven't figured out a great way to handle strings consistently in a C codebase.

bool minischeme_form_is_list(struct minischeme_form* form);
bool minischeme_form_is_number(struct minischeme_form* form);
bool minischeme_form_is_boolean(struct minischeme_form* form);
bool minischeme_form_is_string(struct minischeme_form* form);

bool minischeme_form_symbol_cstring_equal(struct minischeme_toplevel* environment, struct minischeme_form* symbol_form, char* string); 

// Using pointers as optionals... ugh...
struct minischeme_form_string* minischeme_form_as_string(struct minischeme_form* form);
struct minischeme_form_number* minischeme_form_as_number(struct minischeme_form* form);

// Will return 0 or false or whatever the 0 value is for the type if invalid.
int32_t                        minischeme_form_as_integer(struct minischeme_form* form);
float                          minischeme_form_as_real(struct minischeme_form* form);
bool                           minischeme_form_as_boolean(struct minischeme_form* form);

struct minischeme_form_list*   minischeme_form_as_list(struct minischeme_form* form);

#endif
