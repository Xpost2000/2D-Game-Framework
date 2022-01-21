#include <windows.h>

struct file_hentai_file {
    char name[BLACKIRON_MAX_PLATFORM_PATHNAME_LENGTH];

    // this comes from a linked list of buffers. (not blocks, this isn't a memory allocator)
    void*  user_data;
    size_t user_data_length;

    file_hentai_on_file_change_function on_change;
    BY_HANDLE_FILE_INFORMATION          last_win32_file_info;
    BY_HANDLE_FILE_INFORMATION          current_win32_file_info;
};

bool _file_hentai_file_changed(struct file_hentai_file* file);
void _file_hentai_file_update_os_specific_file_information(struct file_hentai_file* file);
void _file_hentai_file_update_os_specific_file_information_first_time(struct file_hentai_file* file);

void _file_hentai_file_update_os_specific_file_information(struct file_hentai_file* file) {
    file->last_win32_file_info = file->current_win32_file_info;

    OFSTRUCT file_structure_dump;
    union {HANDLE handle; uint64_t integer;} file_handle;
    file_handle.integer = OpenFile(file->name, &file_structure_dump, OF_READ);
    GetFileInformationByHandle(file_handle.handle, &file->current_win32_file_info);
    CloseHandle(file_handle.handle);
}

void _file_hentai_file_update_os_specific_file_information_first_time(struct file_hentai_file* file) {
    OFSTRUCT file_structure_dump;
    union {HANDLE handle; uint64_t integer;} file_handle;
    file_handle.integer = OpenFile(file->name, &file_structure_dump, OF_READ);
    {
        GetFileInformationByHandle(file_handle.handle, &file->current_win32_file_info);
        file->last_win32_file_info = file->current_win32_file_info;
    }
    CloseHandle(file_handle.handle);
}

bool _file_hentai_file_changed(struct file_hentai_file* file) {
    BY_HANDLE_FILE_INFORMATION* last_info    = &file->last_win32_file_info;
    BY_HANDLE_FILE_INFORMATION* current_info = &file->current_win32_file_info;

    typedef union stupid_caster {
        FILETIME file_time;
        uint64_t as_uint64_t;
    } stupid_caster;

    stupid_caster last_time    = {last_info->ftLastWriteTime};
    stupid_caster current_time = {current_info->ftLastWriteTime};

    if (last_time.as_uint64_t != current_time.as_uint64_t) {
        return true;
    }

    return false;
}
