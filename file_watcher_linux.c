#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

struct file_watcher_file {
    char name[BLACKIRON_MAX_PLATFORM_PATHNAME_LENGTH];

    // this comes from a linked list of buffers. (not blocks, this isn't a memory allocator)
    void*  user_data;
    size_t user_data_length;

    file_watcher_on_file_change_function on_change;
    struct stat                         last_posix_file_info;
    struct stat                         current_posix_file_info;
};


bool _file_watcher_file_changed(struct file_watcher_file* file);
void _file_watcher_file_update_os_specific_file_information(struct file_watcher_file* file);
void _file_watcher_file_update_os_specific_file_information_first_time(struct file_watcher_file* file);

void _file_watcher_file_update_os_specific_file_information(struct file_watcher_file* file) {
    file->last_posix_file_info = file->current_posix_file_info;
    stat(file->name, &file->current_posix_file_info);
}

void _file_watcher_file_update_os_specific_file_information_first_time(struct file_watcher_file* file) {
    stat(file->name, &file->current_posix_file_info);
    file->last_posix_file_info = file->current_posix_file_info;
}

bool _file_watcher_file_changed(struct file_watcher_file* file) {
    struct stat* last_info    = &file->last_posix_file_info;
    struct stat* current_info = &file->current_posix_file_info;

    uint64_t last_time    = last_info->st_mtime;
    uint64_t current_time = current_info->st_mtime;

    if (last_time != current_time) {
        return true;
    }

    return false;
}

