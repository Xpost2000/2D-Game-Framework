#ifndef STD_ALLOCATOR_H
#define STD_ALLOCATOR_H

// This header defines and implements this allocator!
// requires public/allocator_interface.h
// requires stdlib.h
struct std_allocator {
    struct allocator allocator_base;
};

void* __std_allocator_allocate(struct allocator* allocator, size_t amount) {
    return malloc(amount);
}

void* __std_allocator_reallocate(struct allocator* allocator, void* base, size_t amount) {
    return realloc(base, amount);
}

void __std_allocator_free(struct allocator* allocator, void* base) {
    free(base);
}

struct std_allocator std_allocator_create(void) {
    return (struct std_allocator) {
        .allocator_base = (struct allocator) {
            .allocate   = __std_allocator_allocate,
            .reallocate = __std_allocator_reallocate,
            .free       = __std_allocator_free,
        }
    };
}

#endif
