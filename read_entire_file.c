bool file_exists(char* file_name) {
    FILE* file_handle = fopen(file_name, "rb");
    if (file_handle) {
        fclose(file_handle);
        return 1;
    }
    return 0;
}

size_t get_file_size(char* file_name) {
    FILE* file_handle = fopen(file_name, "rb");
    size_t result = 0;
    if (file_handle) {
        fseek(file_handle, 0, SEEK_END);
        result = ftell(file_handle);
        fseek(file_handle, 0, SEEK_SET);
    }
    fclose(file_handle);
    return result;
}

void read_file_into_buffer(char* file_name, void* buffer, size_t buffer_length) {
    if (file_exists(file_name)) {
        FILE* file_handle = fopen(file_name, "rb");
        size_t read_bytes = fread(buffer, 1, buffer_length, file_handle);
        fclose(file_handle);
    }
}

void* read_entire_file(char* file_name) {
    size_t file_size = get_file_size(file_name);
    char* buffer = system_memory_allocate(file_size+1);

    read_file_into_buffer(file_name, buffer, file_size);
    buffer[file_size] = 0;

    return buffer;
}

void free_entire_file(void* buffer) {
    system_memory_deallocate(buffer);
}
