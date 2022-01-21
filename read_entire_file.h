#ifndef READ_ENTIRE_FILE_H
#define READ_ENTIRE_FILE_H

#include <stdint.h>
#include <stdlib.h>

bool   file_exists(char* file_name);
size_t get_file_size(char* file_name);

void  read_file_into_buffer(char* file_name, void* buffer, size_t buffer_length);
void* read_entire_file(char* file_name);
void  free_entire_file(void* buffer);

#endif
