// Since I'm only using this for small games...
// Me thinks this should be reasonable right?
#define FILE_WATCHER_MAX_TRACKED_FILE_COUNT 32768
struct file_watcher_user_data_block {
    struct file_watcher_user_data_block* previous;
    struct file_watcher_user_data_block* next;
    size_t                              length;
    /*
      Data is implicitly here.
     */
};
struct file_watcher_state {
    size_t                  tracked_file_count;
    struct file_watcher_file tracked_files[FILE_WATCHER_MAX_TRACKED_FILE_COUNT];

    struct file_watcher_user_data_block  sentinel;

    struct file_watcher_user_data_block* head;
    struct file_watcher_user_data_block* tail;

    float timer;
};

static struct file_watcher_state _global_file_watcher = {};

// This only supports push_back, I'm only using this over a dynamic array to not do realloc
// but it doesn't matter.
void* _file_watcher_allocate_and_chain_user_data_block(void* user_data, size_t user_data_length) {
    // create new block and push_back.
    struct file_watcher_user_data_block* new_block = system_memory_allocate(sizeof(struct file_watcher_user_data_block) + user_data_length);

    if (_global_file_watcher.head == &_global_file_watcher.sentinel) {
        _global_file_watcher.head = new_block;
    }

    new_block->next           = &_global_file_watcher.sentinel;
    new_block->previous       = _global_file_watcher.tail;
    new_block->previous->next = new_block;
    _global_file_watcher.tail  = new_block;

    // find where the user data should start and recopy it's contents so it can persist long enough
    // to be used by the callback.
    void* start_of_userdata = (new_block + 1);
    memcpy(start_of_userdata, user_data, user_data_length);
    
    return start_of_userdata;
}

void _file_watcher_on_file_change_function_stub(char* _1, void* _2) {
    console_printf("watcher detects change: %s\n", _1);
}

void file_watcher_initialize(void) {
    console_printf("File watcher startup\n");

    _global_file_watcher.sentinel.next     = &_global_file_watcher.sentinel;
    _global_file_watcher.sentinel.previous = &_global_file_watcher.sentinel;
    _global_file_watcher.sentinel.length   = 0;

    _global_file_watcher.head = &_global_file_watcher.sentinel;
    _global_file_watcher.tail = &_global_file_watcher.sentinel;
}

void _file_watcher_delete_file_userdata(struct file_watcher_file* file) {
    if (file->user_data_length && file->user_data) {
        struct file_watcher_user_data_block* user_data_meta_data = ((void*)file->user_data - sizeof(*user_data_meta_data));
        {
            user_data_meta_data->previous->next = user_data_meta_data->next;
            user_data_meta_data->next->previous = user_data_meta_data->previous;
        }
        system_memory_deallocate(user_data_meta_data);
    }

    file->user_data_length = 0;
    file->user_data        = NULL;
}

void file_watcher_deinitialize(void) {
    console_printf("File watcher die\n");
    for (size_t file_index = _global_file_watcher.tracked_file_count-1; file_index != (size_t)(-1); --file_index) {
        struct file_watcher_file* tracked_file = &_global_file_watcher.tracked_files[file_index];
        _file_watcher_delete_file_userdata(tracked_file);
        --_global_file_watcher.tracked_file_count;
    }

    _global_file_watcher.head = &_global_file_watcher.sentinel;
    _global_file_watcher.tail = &_global_file_watcher.sentinel;
}

void file_watcher_stalk_file(char* file_name, void* user_data, size_t user_data_length, file_watcher_on_file_change_function on_file_change) {
    if (_global_file_watcher.tracked_file_count < FILE_WATCHER_MAX_TRACKED_FILE_COUNT) {
        console_printf("File watcher has started tracking \"%s\" (%p, %d)\n", file_name, user_data, user_data_length);
        struct file_watcher_file* file = &_global_file_watcher.tracked_files[_global_file_watcher.tracked_file_count++];

        if (user_data != NULL && user_data_length > 0) {
            file->user_data        = _file_watcher_allocate_and_chain_user_data_block(user_data, user_data_length);
            file->user_data_length = user_data_length;
        }

        if (on_file_change) {
            file->on_change = on_file_change;
        } else {
            file->on_change = _file_watcher_on_file_change_function_stub;
        }

        strncpy(file->name, file_name, BLACKIRON_MAX_PLATFORM_PATHNAME_LENGTH-1);
        _file_watcher_file_update_os_specific_file_information_first_time(file);
    } else {
        console_printf("File watcher can only track so many files!\n");
    }
}

void file_watcher_stop_stalking_file(char* file_name) {
    for (size_t file_index = _global_file_watcher.tracked_file_count-1; file_index != (size_t)(-1); --file_index) {
        struct file_watcher_file* tracked_file = &_global_file_watcher.tracked_files[file_index];

        if (strncmp(tracked_file->name, file_name, BLACKIRON_MAX_PLATFORM_PATHNAME_LENGTH-1) == 0) {
            _file_watcher_delete_file_userdata(tracked_file);
            _global_file_watcher.tracked_files[file_index] = _global_file_watcher.tracked_files[--_global_file_watcher.tracked_file_count];
            return;
        }
    }
}

void file_watcher_update(float delta_time) {
    if (_global_file_watcher.timer <= 0.0) {
        _global_file_watcher.timer = 0.10;
        for (size_t file_index = 0; file_index < _global_file_watcher.tracked_file_count; ++file_index) {
            struct file_watcher_file* current_file = &_global_file_watcher.tracked_files[file_index];

            _file_watcher_file_update_os_specific_file_information(current_file);

            if (_file_watcher_file_changed(current_file)) {
                current_file->on_change(current_file->name, current_file->user_data);
            }
        }
    } else {
        _global_file_watcher.timer -= delta_time;
    }
}
