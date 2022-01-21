#ifndef STRING_BUILDER_H
#define STRING_BUILDER_H

struct string_builder_node {
    char*  buffer;
    size_t buffer_length;
    struct string_builder_node* next;
};
typedef struct string_builder {
    struct allocator* allocator;
    struct string_builder_node* head;
    struct string_builder_node* tail;

    bool  requires_reconstruction; // I'm a good old rebel, and that's just what I am.
    char* constructed;
} string_builder;

void  string_builder_append_formatted_string(string_builder* string_builder, char* format_string, ...);
void  string_builder_append_cstring(string_builder* string_builder, char* cstring);
void  string_builder_append_string(string_builder* string_builder, char* string, size_t string_length);
void  string_builder_deallocate(string_builder* string_builder);
char* string_builder_construct(string_builder* string_builder);

#endif
