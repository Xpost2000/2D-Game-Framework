#ifndef ALLOCATOR_INTERFACE_H
#define ALLOCATOR_INTERFACE_H

// Basic allocator interface that is implemented by everything I guess.
// should probably be used in single inheritance style if possible.
/*
  IE:
  
  struct chunked_allocator {
     struct allocator interface; // for casting in pointers!
     // specific information.
  };
 */
struct allocator {
    void* (*allocate)(struct allocator*, size_t);
    void* (*reallocate)(struct allocator*, void*, size_t);
    void  (*free)(struct allocator*, void*);
};

// NOTE(jerry): This will cause linkage problems!
//              if included in different translation units! Fuck!
void* allocator_allocate_null(struct allocator* _allocator, size_t _amount) {
    return 0;
}

void* allocator_reallocate_null(struct allocator* _allocator, void* _base, size_t _amount) {
    return 0;
}

void allocator_free_null(struct allocator* _allocator, void* _base) {
    
}

#endif
