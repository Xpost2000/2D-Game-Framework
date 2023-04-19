struct file_watcher_file {
    char name[BLACKIRON_MAX_PLATFORM_PATHNAME_LENGTH];

    // this comes from a linked list of buffers. (not blocks, this isn't a memory allocator)
    void*  user_data;
    size_t user_data_length;

    file_watcher_on_file_change_function on_change;
};
// Compile this in if I haven't written an implementation yet.
bool _file_watcher_file_changed(struct file_hentai_file* file) {
    
}
void _file_watcher_file_update_os_specific_file_information(struct file_watcher_file* file) {
    
}
void _file_watcher_file_update_os_specific_file_information_first_time(struct file_watcher_file* file) {
    
}
