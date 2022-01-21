/*
  I mean this isn't a very serious codebase, and I'm a teenager.
  
  So let's have some fun as well!
 */
#ifndef FILE_HENTAI_H
#define FILE_HENTAI_H

// For now file hentai will just expose itself with a callback system
// although I considered having an event queue, although I'm not really sure how
// useful that would be for my use-case which is exclusively hotreloading where callbacks... Would be fine?

// Also this is going to be single threaded, although if you replace some stuff with atomics this should
// probably be fine running on another thread.

typedef void (*file_hentai_on_file_change_function)(char*, void*);

void file_hentai_initialize(void);
void file_hentai_update(float delta_time);
void file_hentai_deinitialize(void);

void file_hentai_stalk_file(char* file_name, void* user_data, size_t user_data_length, file_hentai_on_file_change_function on_file_change);
void file_hentai_stop_stalking_file(char* file_name);

#endif
