#ifndef FILE_WATCHER_H
#define FILE_WATCHER_H

// For now file watcher will just expose itself with a callback system
// although I considered having an event queue, although I'm not really sure how
// useful that would be for my use-case which is exclusively hotreloading where callbacks... Would be fine?

// Also this is going to be single threaded, although if you replace some stuff with atomics this should
// probably be fine running on another thread.

typedef void (*file_watcher_on_file_change_function)(char*, void*);

void file_watcher_initialize(void);
void file_watcher_update(float delta_time);
void file_watcher_deinitialize(void);

void file_watcher_stalk_file(char* file_name, void* user_data, size_t user_data_length, file_watcher_on_file_change_function on_file_change);
void file_watcher_stop_stalking_file(char* file_name);

#endif
