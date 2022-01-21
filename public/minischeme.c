/* #error "implement sprite functionality stuff!" */
struct minischeme_lexer_state {
    char* string;
    size_t string_length;
    size_t index;
    bool fail;
};
bool minischeme_lexer_finished(struct minischeme_lexer_state* lexer) {
    return (lexer->index >= lexer->string_length) || lexer->fail;
}
char minischeme_lexer_peek(struct minischeme_lexer_state* lexer) {
    return lexer->string[lexer->index];
}

char minischeme_lexer_eat(struct minischeme_lexer_state* lexer) {
    return lexer->string[lexer->index++];
}

// attempt to parse this in place.
// we assume the files are only utf8
// TODO(jerry): error checking.
static uint32_t _minischeme_toplevel_intern_or_reuse_symbol(struct minischeme_toplevel* toplevel, char* string, size_t string_length) {
    uint64_t hash_key   = fnv1a_hash(string, string_length);
    uint64_t hash_index = hash_key & (MINISCHEME_SYMBOL_TABLE_LENGTH-1);

    {
        struct minischeme_form_string* symbol = &toplevel->symbol_table[hash_index];

        // NOTE(jerry):
        // this CAN, and WILL infinite loop if we run out of symbols to store.
        // watch this!

        if (symbol->length && strncmp(string, symbol->string, MINISCHEME_FORM_STRING_MAX_LENGTH) != 0) {
            // linear probing.
            while (symbol->length) {
                hash_index++;
                hash_index &= (MINISCHEME_SYMBOL_TABLE_LENGTH - 1);
                symbol = &toplevel->symbol_table[hash_index];
            }
        } else if (symbol->length == 0) {
            symbol->length = string_length;
            strncpy(symbol->string, string, MINISCHEME_FORM_STRING_MAX_LENGTH-1);
        }
    }

    return hash_index;
}

// environments will not have symbol tables, environment just have bindings.
struct minischeme_form _minischeme_make_form(struct minischeme_toplevel* toplevel, uint8_t form_type, char* string, size_t string_length) {
    struct minischeme_form result = {};
    result.type = form_type;

    switch (form_type) {
        case MINISCHEME_FORM_SYMBOL: {
            result.symbol.index = _minischeme_toplevel_intern_or_reuse_symbol(toplevel, string, string_length);
        } break;

        case MINISCHEME_FORM_STRING: {
            result.string.length = string_length;
            size_t copy_length = string_length;
            
            if (copy_length > MINISCHEME_FORM_STRING_MAX_LENGTH) {
                copy_length = MINISCHEME_FORM_STRING_MAX_LENGTH;
            }

            strncpy(result.string.string, string, MINISCHEME_FORM_STRING_MAX_LENGTH-1);
        } break;
            
        case MINISCHEME_FORM_NUMBER: {
            uint8_t number_type = MINISCHEME_FORM_NUMBER_INTEGER;

            for (size_t index = 0; index < string_length; ++index) {
                if (string[index] == '.') {
                    number_type = MINISCHEME_FORM_NUMBER_REAL;
                    break;
                }
            }

            result.number.number_type = number_type;
            if (number_type == MINISCHEME_FORM_NUMBER_REAL) {
                result.number.real    = atof(string);
            } else {
                result.number.integer = atoi(string);
            }

        } break;
    }

    return result;
}


bool minischeme_lexer_read_form(struct minischeme_toplevel* toplevel, struct minischeme_lexer_state* lexer, struct minischeme_form* result);

size_t minischeme_form_list_length(struct minischeme_form* list_form) {
    if (list_form && list_form->type == MINISCHEME_FORM_LIST) {
        struct minischeme_form_list* list = list_form->list;
        size_t count = 0;

        while (list) {
            list = list->cdr;
            count++;
        }

        return count;
    }

    return 0;
}

struct minischeme_form* minischeme_form_list_elt(struct minischeme_form* list, size_t index) {
    if (list && list->type == MINISCHEME_FORM_LIST) {
        size_t length = minischeme_form_list_length(list);

        if (index >= length) {
            return NULL;
        }

        struct minischeme_form_list* cursor = list->list;
        size_t iterated = 0;

        while (cursor && iterated != index) {
            cursor = cursor->cdr;
            iterated++;
        }

        if (cursor) {
            return &cursor->car;
        }
    }

    return NULL;
}

struct minischeme_form* minischeme_form_list_last(struct minischeme_form* list) {
    struct minischeme_form* result = minischeme_form_list_elt(list, minischeme_form_list_length(list) - 1);
    return result;
}
struct minischeme_form* minischeme_form_list_first(struct minischeme_form* list) {
    struct minischeme_form* result = minischeme_form_list_elt(list, 0);
    return result;
}

// NOTE(jerry): allocation festival.
void minischeme_list_push(struct minischeme_form_list* cons_cell, struct minischeme_form item) {
    if (cons_cell->cdr) {
        minischeme_list_push(cons_cell->cdr, item);
    } else {
        cons_cell->cdr      = system_memory_allocate(sizeof(*cons_cell->cdr));
        cons_cell->cdr->car = item;
        cons_cell->cdr->cdr = NULL;
    }
}
void minischeme_form_list_push(struct minischeme_form* cons_cell, struct minischeme_form item) {
    if (cons_cell->list) {
        minischeme_list_push(cons_cell->list, item);
    } else {
        cons_cell->list      = system_memory_allocate(sizeof(*cons_cell->list));
        cons_cell->list->car = item;
        cons_cell->list->cdr = NULL;
    }
}

struct minischeme_form _minischeme_parse_and_build_list(struct minischeme_toplevel* toplevel, char* string, size_t string_length) {
    struct minischeme_form result = {};
    result.type = MINISCHEME_FORM_LIST;

    struct minischeme_lexer_state lexer = (struct minischeme_lexer_state) {
        .string = string,
        .string_length = string_length,
        .index = 0,
        .fail = false,
    };
    
    while (!minischeme_lexer_finished(&lexer)) {
        struct minischeme_form read_form = {};
        bool able_to_find_form = minischeme_lexer_read_form(toplevel, &lexer, &read_form);

        if (able_to_find_form) {
            // NOTE(jerry):
            //
            // Converting this to a vector would be trivial
            // even though this works recursively. So I can just do that.
            // I'm not so thrilled about the fact this will still be an allocation
            // festival (it's just not nearly as intense)... Yikes!
            //

            // might be a little iffy and not match real lisp semantics.
            minischeme_form_list_push(&result, read_form);
        }
    }

    return result;
}

bool minischeme_lexer_read_form(struct minischeme_toplevel* toplevel, struct minischeme_lexer_state* lexer, struct minischeme_form* result) {
    while (!minischeme_lexer_finished(lexer)) {
        char current = minischeme_lexer_peek(lexer);

        if (is_whitespace_character(current)) {
            minischeme_lexer_eat(lexer);
            continue;
        }

        switch (current) {
            case ';': {
                // Read comment.
                while (!minischeme_lexer_finished(lexer)) {
                    if (minischeme_lexer_peek(lexer) == '\n') {
                        break;
                    }

                    minischeme_lexer_eat(lexer);
                }
                return false;
            } break;
                
            // read list
            case '(': {
                minischeme_lexer_eat(lexer);

                // if I pass lexer state, this weird thing isn't really needed lol.
                char* start_of_list_form = lexer->string + lexer->index;
                char* end_of_list_form   = start_of_list_form;

                int32_t brace_count = 1;
                while (!minischeme_lexer_finished(lexer) && brace_count) {
                    if ((*end_of_list_form) == ')') {
                        brace_count--;
                    } else if ((*end_of_list_form) == '(') {
                        brace_count++;
                    }
                    
                    end_of_list_form++;
                    minischeme_lexer_eat(lexer);
                }

                end_of_list_form--;         // reject closing parenthesis
                minischeme_lexer_eat(lexer);

                size_t inner_length = (end_of_list_form - start_of_list_form);

                struct minischeme_form new_form = _minischeme_parse_and_build_list(toplevel, start_of_list_form, inner_length);
                *result = new_form;
                return true;
            } break;

                // try to read identifier or string
            default: {
                char* start_of_token = lexer->string + lexer->index;
                bool is_string = ((*start_of_token)) == '\"';

                if (is_string) {
                    start_of_token++;
                    minischeme_lexer_eat(lexer);
                }

                char* end_of_token   = start_of_token;

                while (!minischeme_lexer_finished(lexer)) {
                    if (is_string) {
                        if ((*end_of_token) == '\"') {
                            break;
                        }
                    } else {
                        if (is_whitespace_character((*end_of_token))) {
                            break;
                        }
                    }
                    end_of_token++;
                    minischeme_lexer_eat(lexer);
                }

                if (is_string) {
                    minischeme_lexer_eat(lexer);
                }

                size_t inner_length = (end_of_token - start_of_token);
                    
                uint8_t form_type = MINISCHEME_FORM_SYMBOL;
                {
                    if (is_string) {
                        form_type = MINISCHEME_FORM_STRING;
                    } else {
                        bool is_number = true;

                        for (size_t inner_index = 0; inner_index < inner_length && is_number; ++inner_index) {
                            if (!(is_numeric_character(start_of_token[inner_index]) || start_of_token[inner_index] == '.')) {
                                is_number = false;
                            }
                        }

                        if (is_number) {
                            form_type = MINISCHEME_FORM_NUMBER;
                        }
                    }
                }

                struct minischeme_form new_form = _minischeme_make_form(toplevel, form_type, start_of_token, inner_length);
                *result = new_form;
                
                return true;
            } break;
        }
        
        minischeme_lexer_eat(lexer);
        return false;
    }
}

// string table does not grow!
// it just doesn't fit on the stack!
struct minischeme_toplevel _minischeme_create_toplevel(void) {
    struct minischeme_toplevel result = {};
    result.symbol_table = system_memory_allocate(sizeof(*result.symbol_table) * MINISCHEME_SYMBOL_TABLE_LENGTH);
    return result;
}

struct minischeme_toplevel minischeme_toplevel_load_from_string(char* string, size_t string_length) {
    struct minischeme_toplevel result = _minischeme_create_toplevel();
    struct minischeme_lexer_state lexer = (struct minischeme_lexer_state) {
        .string = string,
        .string_length = string_length,
        .index = 0,
        .fail = false,
    };

    while (!minischeme_lexer_finished(&lexer)) {
        struct minischeme_form read_form = {};
        bool able_to_find_form = minischeme_lexer_read_form(&result, &lexer, &read_form);

        if (able_to_find_form) {
            // link it on.
            dynamic_array_push(minischeme_form)(&result.forms, read_form);
        }
    }
    
    return result;
}

struct minischeme_toplevel minischeme_toplevel_load_from_cstring(char* string) {
    return minischeme_toplevel_load_from_string(string, strlen(string));
}

struct minischeme_toplevel minischeme_toplevel_load_from_file(struct system_api* system, char* file_location) {
    struct minischeme_toplevel result             = {};
    size_t                     file_buffer_length = system->file_size(file_location);
    char*                      file_buffer        = system->read_entire_file(file_location);

    result = minischeme_toplevel_load_from_string(file_buffer, file_buffer_length);

    system->free_file_buffer(file_buffer);
    return result;
}

struct minischeme_form_string minischeme_toplevel_symbol_name_from_form(struct minischeme_toplevel* toplevel, struct minischeme_form form) {
    if (form.type == MINISCHEME_FORM_SYMBOL) {
        return toplevel->symbol_table[form.symbol.index];
    }

    return (struct minischeme_form_string){};
}

static void minischeme_list_free(struct minischeme_form_list* list) {
    struct minischeme_form_list* cursor = list;
    
    while (cursor) {
        if (cursor->car.type == MINISCHEME_FORM_LIST && cursor->car.list) {
            minischeme_list_free(cursor->car.list);
        }

        struct minischeme_form_list* next = cursor->cdr;
        system_memory_deallocate(cursor);
        cursor = next;
    }
}

void minischeme_toplevel_free(struct minischeme_toplevel* instance) {
    for (size_t form_index = 0; form_index < instance->forms.count; ++form_index) {
        struct minischeme_form* current_form = &instance->forms.data[form_index];

        if (current_form->type == MINISCHEME_FORM_LIST && current_form->list) {
            minischeme_list_free(current_form->list);
        }
    }

    system_memory_deallocate(instance->symbol_table);
    dynamic_array_free(minischeme_form)(&instance->forms);
}

bool minischeme_form_is_list(struct minischeme_form* form) {
    if (form->type == MINISCHEME_FORM_LIST) {
        return true;
    }

    return false;
}

bool minischeme_form_is_number(struct minischeme_form* form) {
    if (form->type == MINISCHEME_FORM_NUMBER) {
        return true;
    }

    return false;
}

bool minischeme_form_is_boolean(struct minischeme_form* form) {
    if (form->type == MINISCHEME_FORM_TRUE || form->type == MINISCHEME_FORM_FALSE) {
        return true;
    }

    return false;
}

bool minischeme_form_is_string(struct minischeme_form* form) {
    if (form->type == MINISCHEME_FORM_STRING) {
        return true;
    }

    return false;
}
    
bool minischeme_form_symbol_cstring_equal(struct minischeme_toplevel* environment, struct minischeme_form* symbol_form, char* string) {
    if (symbol_form && symbol_form->type == MINISCHEME_FORM_SYMBOL) {
        struct minischeme_form_string* interned_string = &environment->symbol_table[symbol_form->symbol.index];
        char* symbol_string = interned_string->string;
        int comparison_result = strncmp(symbol_string, string, interned_string->length);
        console_printf("%d (%.*s, %s) compare\n", comparison_result, interned_string->length, symbol_string, string);
        return comparison_result == 0;
    }

    return false;
}


struct minischeme_form_string* minischeme_form_as_string(struct minischeme_form* form) {
    if (form->type != MINISCHEME_FORM_STRING) {
        return NULL;
    }

    return &form->string;
}
struct minischeme_form_number* minischeme_form_as_number(struct minischeme_form* form) {
    if (form->type != MINISCHEME_FORM_NUMBER) {
        return NULL;
    }

    return &form->number;
}

int32_t minischeme_form_as_integer(struct minischeme_form* form) {
    if (form->type == MINISCHEME_FORM_NUMBER && form->number.number_type == MINISCHEME_FORM_NUMBER_INTEGER) {
        return form->number.integer;
    }

    return 0;
}
float minischeme_form_as_real(struct minischeme_form* form) {
    if (form->type == MINISCHEME_FORM_NUMBER && form->number.number_type == MINISCHEME_FORM_NUMBER_REAL) {
        return form->number.real;
    }

    return 0.0;
}
bool minischeme_form_as_boolean(struct minischeme_form* form) {
    if (form->type == MINISCHEME_FORM_TRUE) {
        return true;
    }

    return false;
}

struct minischeme_form_list*   minischeme_form_as_list(struct minischeme_form* form) {
    if (form->type != MINISCHEME_FORM_LIST) {
        return NULL;
    }

    return form->list;
}

#define type int
#include "dynamic_array.h"

void __testing_basic_read(void) {
    {
        // sanity check
        struct dynamic_array(int) w = {};
        dynamic_array_push(int)(&w, 3);
        dynamic_array_push(int)(&w, 3);

        dynamic_array_free(int)(&w);
        printf("leak check\n");
        
        _system_memory_leak_check();
    }
    
    // blam
    #if 0
    {
        /* struct minischeme_toplevel asdf = minischeme_toplevel_load_from_cstring("1 2 3 4 5"); */
        /* struct minischeme_toplevel asdf = minischeme_toplevel_load_from_cstring("1 2 3 4 if \"sad dingle berries\" (1 2 3 4 (1 2 3 4))"); */
        struct minischeme_toplevel asdf = minischeme_toplevel_load_from_cstring("1 (1) (3 4 ((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((((()))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))))5 (2))");
        #if 0
        printf("test man\n");
        for (size_t form_index = 0; form_index < asdf.forms.count; ++form_index) {
            struct minischeme_form* form = &asdf.forms.data[form_index];

            switch (form->type) {
                case MINISCHEME_FORM_SYMBOL: {
                    struct minischeme_form_string symbol_name = minischeme_toplevel_symbol_name_from_form(&asdf, *form);
                    printf("SYMBOL: %.*s\n", symbol_name.length, symbol_name.string);
                } break;
                case MINISCHEME_FORM_STRING: {
                    printf("STRING: %.*s\n", form->string.length, form->string.string);
                } break;
                case MINISCHEME_FORM_NUMBER: {
                    if (form->number.number_type == MINISCHEME_FORM_NUMBER_INTEGER) {
                        printf("INTEGER NUMBER: %d\n", form->number.integer);
                    } else {
                        printf("REAL NUMBER: %f\n", form->number.real);
                    }
                } break;
                case MINISCHEME_FORM_LIST: {
                    printf("not printing list\n");
                } break;
                default: {
                } break;
            }
        }
        #endif

        minischeme_toplevel_free(&asdf);
        _system_memory_leak_check();
    }
    #endif
}
