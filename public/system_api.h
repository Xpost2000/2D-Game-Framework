#ifndef SYSTEM_API_H
#define SYSTEM_API_H

// This handles OS/platform/system code or whatever the fuck.
// Also handles some stateful things in common.h
// I need to put common in subdirectory, since this is really "engine_common",
// not common, which is actually used for the game...

// system_api should also have a system allocator option
// so we can give it to the dynamic_arrays if we create them as otherwise we'd be
// using a different type of malloc lol.
struct system_api {
    void*  (*memory_allocate)(size_t);
    void   (*memory_deallocate)(void*);
    void*  (*memory_reallocate)(void*, size_t);
    size_t (*memory_allocated_total)(void);

    // at least this let's you use any C compiler...
    uint64_t (*read_timestamp_counter)(void);

    void (*set_fixed_framerate_update)(int32_t);

    float (*average_frametime)(void);
    float (*average_framerate)(void);

    void (*set_window_title)(char*);
    void (*set_window_resolution)(int32_t, int32_t);
    void (*set_window_resizable)(bool);
    void (*set_window_fullscreen)(bool);

    // TODO(jerry): read file.
    bool   (*file_exists)(char*);
    size_t (*file_size)(char*);
    void*  (*read_entire_file)(char*);
    void   (*read_file_into_buffer)(char*, void*, size_t);
    void   (*free_file_buffer)(void*);
};

#endif
