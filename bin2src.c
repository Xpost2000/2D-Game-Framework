/*
  A stupid C program that does what xxd does with
  the source option just incase you don't have it installed
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct filebuffer {
    char*  data;
    size_t length;
};
struct filebuffer filebuffer(const char* path) {
    char* data;
    size_t length;

    FILE* f = fopen(path, "rb+");

    fseek(f, 0, SEEK_END);
    length = ftell(f);
    fseek(f, 0, SEEK_SET);

    data = malloc(length);
    fread(data, 1, length, f);

    fclose(f);

    return (struct filebuffer){
        .data = data,
        .length = length
    };
}

// don't do pretty printing... It's just data.
void spit_buffer(FILE* stream, char* variable_name, struct filebuffer buffer) {
    fprintf(stream, "struct {\n");
    fprintf(stream, "\tunsigned char buffer[%d];\n", buffer.length);
    fprintf(stream, "\tsize_t length;\n");
    fprintf(stream, "} %s = {\n", variable_name);
    printf("\t{");
    for (size_t i = 0; i < buffer.length; ++i) {
        if ((i % 20) == 0) {
            printf("\n\t\t");
        }
        printf("0x%X, ", buffer.data[i] & 0xFF);
    }
    printf("\n\t},\n\t%d", buffer.length);
    fprintf(stream, "\n};\n\n");
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "feedme files\n");
        return 1;
    }

    FILE* output = stdout;
    fprintf(output, "// This file was generated. Please do not touch!\n");
    fprintf(output, "/*\n\tORIGINAL FILES:\n");
    for (int i = 1; i < argc; ++i) {
        fprintf(output, "\t\t%s\n", argv[i]);
    }
    fprintf(output, "*/\n\n");
    char temp[1024];
    for (int i = 1; i < argc; ++i) {
        char* filename = argv[i];
        strncpy(temp, "FileData_", 1024);
        strncpy(temp+9, filename, 1024-9);
        int len = strlen(temp);
        for (int j = 0; j < len; ++j) {
            if (temp[j] == '/' || temp[j] == '\\' || temp[j] == ':' || temp[j] == ' ' || temp[j] == '.' || temp[j] == ',' || temp[j] == '-' || temp[j] == '+' || temp[j] == '=') {
                temp[j] = '_';
            }
        }
        spit_buffer(output, temp, filebuffer(filename));
    }
    fclose(output);

    return 0;
}
