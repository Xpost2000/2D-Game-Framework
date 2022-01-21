// This is a small bootstrapping build program
// it's a normal executable and not a batchscript cause why not?

// This is only required because on Windows they lock files
// while they're loaded. While this is good behavior, it's kind
// of annoying when doing things like DLL reloading or replacing executables
// at runtime like this...

// gcc build_bootstrapper.c -o build

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h> // variadic argument usage.
#include <stdlib.h>

#include <stdbool.h>
#include <stddef.h>

#include <string.h>

#include "public/allocator_interface.h"
#include "public/std_allocator.h"

#include "public/string_builder.h"
#include "public/string_builder.c"

#if defined(_WIN32)
#include "os_process_win32.c"
#elif defined(__linux__)
#include "os_process_linux.c"
#endif

// TODO(jerry): This was copied and pasted
// This should be moved into the engine...
enum operating_system {
    OPERATING_SYSTEM_WINDOWS,
    OPERATING_SYSTEM_LINUX,
    OPERATING_SYSTEM_MAC,
};

static int operating_system =
#if defined(_WIN32)
    OPERATING_SYSTEM_WINDOWS
#elif defined(__linux__)
    OPERATING_SYSTEM_LINUX
#endif
    ;

static char* shared_object_extension(void) {
    switch (operating_system) {
        case OPERATING_SYSTEM_WINDOWS: {
            return ".dll";
        } break;
        case OPERATING_SYSTEM_LINUX: {
            return ".so";
        } break;
    }

    return ".out";
}

static char* executable_extension(void) {
    switch (operating_system) {
        case OPERATING_SYSTEM_WINDOWS: {
            return ".exe";
        } break;
        case OPERATING_SYSTEM_LINUX: {
            return ".x86_64";
        } break;
    }

    return ".out";
}

#if defined(_WIN32)
#include <windows.h>
uint64_t platform_file_get_last_modified_time(char* file_name) {
    OFSTRUCT                   file_structure_dump;
    BY_HANDLE_FILE_INFORMATION file_info;

    union {HANDLE handle; uint64_t integer;} file_handle;
    file_handle.integer = OpenFile(file_name, &file_structure_dump, OF_READ);

    GetFileInformationByHandle(file_handle.handle, &file_info);

    CloseHandle(file_handle.handle);

    typedef union stupid_caster {
        FILETIME file_time;
        uint64_t as_uint64_t;
    } stupid_caster;

    stupid_caster time = { file_info.ftLastWriteTime };
    return time.as_uint64_t;
}
#elif defined(__linux__)
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
uint64_t platform_file_get_last_modified_time(char* file_name) {
    struct stat file_info;
    stat(file_name, &file_info);
    return file_info.st_mtime;
}
#endif

static struct std_allocator _global_std_allocator = {};
int main(int argument_count, char** argument_values) {
    _global_std_allocator = std_allocator_create();

    bool requires_rebuilding = false;
    {
        char* engine_executable_name;

        if (operating_system == OPERATING_SYSTEM_LINUX) {
            engine_executable_name = "engine_build";
        } else {
            engine_executable_name = "engine_build.exe";
        }

        uint64_t build_executable_file_time = platform_file_get_last_modified_time(engine_executable_name);
        uint64_t buildtool_source_file_time = platform_file_get_last_modified_time("blackiron_build_tool.c");

        if (build_executable_file_time < buildtool_source_file_time) {
            requires_rebuilding = true;
            fprintf(stderr, "Rebuilding engine!\n");
            fflush(stderr);
        }
    }

    bool build_success = false;

    if (requires_rebuilding) {
        if (os_process_shell_start_and_run_synchronously("clang blackiron_build_tool.c -std=c11 -I./. -O2 -o engine_build") != 0)  {
            build_success = false;
        } else {
            build_success = true;
        }
    } else {
        build_success = true;
    }

    if (!build_success) {
        fprintf(stderr, "build script error!");
        return 1;
    } else {
        fprintf(stderr, "============== Engine build.... Engage!\n");
        string_builder build_command = { .allocator = &_global_std_allocator };

#if defined(__linux__)
        string_builder_append_cstring(&build_command, "./engine_build");
#else
        string_builder_append_cstring(&build_command, "./engine_build.exe");
#endif

        for (size_t argument_index = 1; argument_index < argument_count; ++argument_index) {
            string_builder_append_cstring(&build_command, " ");
            string_builder_append_cstring(&build_command, argument_values[argument_index]);
        }

        fprintf(stderr, "build-command: (%s),\n", string_builder_construct(&build_command));
        fflush(stderr);
        if (os_process_shell_start_and_run_synchronously(string_builder_construct(&build_command)) != 0) {
            return 1;
        } else {
        }
    }

    return 0;
}
