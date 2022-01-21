static struct string_builder_node* _string_builder_allocate_new_link(string_builder* string_builder, char* buffer, size_t buffer_length) {
    struct string_builder_node* result = string_builder->allocator->allocate(string_builder->allocator, sizeof(struct string_builder_node));
    memset(result, 0, sizeof(*result));
    result->buffer        = buffer;
    result->buffer_length = buffer_length;
    result->next          = NULL;
    return result;
}

static void _string_builder_push_link(string_builder* string_builder, struct string_builder_node* new_link) {
    if (string_builder->head == NULL) {
        string_builder->head = new_link;
    }

    if (string_builder->tail) {
        string_builder->tail->next = new_link;
    }

    string_builder->tail = new_link;
    string_builder->requires_reconstruction = true;
}

void string_builder_deallocate(string_builder* string_builder) {
    if (string_builder->constructed) {
        free(string_builder->constructed);
        string_builder->constructed = NULL;
    }

    struct string_builder_node* cursor = string_builder->head;

    while (cursor) {
        struct string_builder_node* next = cursor->next;
        string_builder->allocator->free(string_builder->allocator, cursor);
        cursor = next;
    }

    string_builder->head = NULL;
    string_builder->tail = NULL;
}

void string_builder_append_cstring(string_builder* string_builder, char* cstring) {
    size_t string_length = 0;
    {
        char* cursor = cstring;
        while (*(++cursor));
        string_length = cursor - cstring;
    }

    struct string_builder_node* new_link = _string_builder_allocate_new_link(string_builder, cstring, string_length);
    _string_builder_push_link(string_builder, new_link);
}

void string_builder_append_string(string_builder* string_builder, char* string, size_t string_length) {
    struct string_builder_node* new_link = _string_builder_allocate_new_link(string_builder, string, string_length);
    _string_builder_push_link(string_builder, new_link);
}

int s(char* format_string, ...) {
    int e;
    {
        va_list variadic_arguments_list;
        va_start(variadic_arguments_list, format_string);
        e = vsnprintf(NULL, 0, format_string, variadic_arguments_list);
        va_end(variadic_arguments_list);
    }

    return e;
}
void string_builder_append_formatted_string(string_builder* string_builder, char* format_string, ...) {
    size_t string_length = 0;
    {
        va_list variadic_arguments_list;
        va_start(variadic_arguments_list, format_string);
        vsnprintf(NULL, 0, format_string, variadic_arguments_list);
        va_end(variadic_arguments_list);
    }
    {
        va_list variadic_arguments_list;
        va_start(variadic_arguments_list, format_string);
        // does not include null terminator.
        string_length = vsnprintf(NULL, 0, format_string, variadic_arguments_list);
        va_end(variadic_arguments_list);
    }

    // add extra byte for null termination.
    char* new_string_buffer = string_builder->allocator->allocate(string_builder->allocator, string_length+1);
    memset(new_string_buffer, 0, string_length+1);
    {
        va_list variadic_arguments_list;
        va_start(variadic_arguments_list, format_string);

        vsnprintf(new_string_buffer, string_length+1, format_string, variadic_arguments_list);
        va_end(variadic_arguments_list);
    }

    struct string_builder_node* new_link = _string_builder_allocate_new_link(string_builder, new_string_buffer, string_length);
    _string_builder_push_link(string_builder, new_link);
}

char* string_builder_construct(string_builder* string_builder) {
    // Cache the string we construct.
    // So we can avoid extra allocations. Of course... You are promising not to modify this
    // right? Strings are immutable... RIGHT???
    if (string_builder->requires_reconstruction) {
        if (string_builder->constructed) {
            free(string_builder->constructed);
        }

        // Double list traversal to get a simpler one pass allocation for the string size.
        // NOTE(jerry):
        // The string builder should just keep track of this itself when adding new nodes.
        // String builders are usually very temporary(at least that's how I'd use them...), so we don't even need to have functionality to remove nodes.
        size_t total_length = 0;
        {
            struct string_builder_node* cursor = string_builder->head;
            while (cursor) {
                total_length += cursor->buffer_length;
                cursor = cursor->next;
            }
        }

        char* constructed_string = string_builder->allocator->allocate(string_builder->allocator, total_length+1);
        memset(constructed_string, 0, total_length+1);
        size_t write_index = 0;
        {
            struct string_builder_node* cursor = string_builder->head;
            while (cursor) {
                // NOTE(jerry):
                // *sigh*... I made all of the nodes themselves C strings...
                // NULL termination and NULL references/pointers were a mistake.
                for (size_t character_index = 0; character_index < cursor->buffer_length; ++character_index) {
                    constructed_string[write_index++] = cursor->buffer[character_index];
                }
                cursor = cursor->next;
            }
        }
        constructed_string[total_length] = 0;
        string_builder->constructed = constructed_string;
    }

    return string_builder->constructed;
}
