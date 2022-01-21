// I hate the syntax of multi-line macros,
// so this is my alternative. Although EMACS can deal with it a bit
// more sanely than other editors I've found...

// These get long so this is in a different style since it's slightly harder to
// read than usual...

// No I'm not happy with this either, but I'm not using C++, so this is really
// the best trick I have.

// I don't like that most stretchy buffers can change their pointer from underneath
// (even though I'm aware in most cases that's actually fine since you seldom store actual
// references to vectors, and only require references in procedures.)

// I also don't like the void pointer method since it's limiting and tricky as hell to do.
// NOTE(jerry): Mildly interested as to why this can't be inlined?
// I need to at least make the last memcpy be inline-able array->data[array->count++].
// the regrow check is most likely causing the inability to inline, I could use a macro to force
// the simple part to be inlined, but I'm probably not going to do it.
// Since I'll limit my actual dynamic array usage heavily anyways... push time is inconsequential
// to me for now though. It still iterates contiguously which is the most important thing.

#include <stddef.h>

// type tags are a bit more annoying... Since I write C using the tags
// instead of using typedef struct (except in peculiar cases like handle types.)
#ifdef type_tag
#define fully_qualified_type_name type_tag type
#else
#define fully_qualified_type_name type
#endif
struct dynamic_array(type) {
    // TODO(jerry): specify an allocator
    size_t count;
    size_t capacity;
    fully_qualified_type_name* data;
};

static struct dynamic_array(type)
template_fn(dynamic_array_new)(void) {
    return (struct dynamic_array(type)) {
        .count = 0,
        .capacity = 0,
        .data = 0,
    };
}

static void
template_fn(dynamic_array_free)(struct dynamic_array(type)* array) {
    if (array->data) {
        array->count = 0;
        array->capacity = 0;
        system_memory_deallocate(array->data); 
        array->data = 0;
    }
}

static void
template_fn(dynamic_array_reserve)(struct dynamic_array(type)* array, size_t amount) {
    if (array->data) {
        void* realloced_data = system_memory_reallocate(array->data, amount * sizeof(fully_qualified_type_name));
        array->data = realloced_data;
    } else {
        array->data = system_memory_allocate(amount * sizeof(fully_qualified_type_name));
    }

    array->capacity = amount;
}

static void
template_fn(dynamic_array_push)(struct dynamic_array(type)* array, fully_qualified_type_name value) {
    if (array->data && array->count+1 >= array->capacity) {
        // grow into powers of two, which works fine enough.
        size_t new_capacity = (size_t)(array->capacity * 2);
        void* realloced_data = system_memory_reallocate(array->data, new_capacity * sizeof(fully_qualified_type_name));

        assertion(realloced_data);

        array->capacity = new_capacity;
        array->data = realloced_data;
    } else if (array->data == NULL) {
        array->data = system_memory_allocate(sizeof(fully_qualified_type_name) * 1);
        array->capacity = 1;
    }

    array->data[array->count++] = value;
}

static void
template_fn(dynamic_array_pop)(struct dynamic_array(type)* array) {
    array->count--;
}

static fully_qualified_type_name
template_fn(dynamic_array_pop_and_retrieve_last)(struct dynamic_array(type)* array) {
    fully_qualified_type_name result = array->data[array->count-1];
    array->count--;
    return result;
}

static void
template_fn(dynamic_array_erase)(struct dynamic_array(type)* array, size_t index) {
    array->data[index] = array->data[--array->count];
}

static void
template_fn(dynamic_array_erase_ordered)(struct dynamic_array(type)* array, size_t index) {
    for (; index < array->count-1; ++index) {
        array->data[index] = array->data[index+1];
    }

    array->count -= 1;
}

static fully_qualified_type_name
template_fn(dynamic_array_erase_and_retrieve)(struct dynamic_array(type)* array, size_t index) {
    fully_qualified_type_name result = array->data[index];
    array->data[index] = array->data[--array->count];
    return result;
}

static struct dynamic_array(type)
template_fn(dynamic_array_duplicate)(struct dynamic_array(type)* array) {
    size_t count = array->count;
    struct dynamic_array(type) result = dynamic_array_new(type)();
    dynamic_array_reserve(type)(&result, count);

    for (size_t index = 0; index < count; ++index) {
        result.data[index] = array->data[index];
    }
        
    return result;
}

static void
template_fn(dynamic_array_copy)(struct dynamic_array(type)* array, size_t start, size_t end, fully_qualified_type_name* destination, size_t destination_length) {
    size_t copied = 0;
    for (size_t index = start; index != end && copied < destination_length; ++index) {
        destination[copied++] = array->data[index];
    }
}

#undef type
#undef type_tag
#undef fully_qualified_type_name
