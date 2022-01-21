#include <stdio.h>
#include <stdlib.h>

#include <stdbool.h>
#include <stdarg.h> // variadic argument usage.
#include <stddef.h>

#include "public/allocator_interface.h"
#include "public/std_allocator.h"

// This should only run within the engine source directory (for obvious reasons...)

// This is a scuffed build tool, cause reasons.
// It's basically a glorified batch file that you build with C.

// This is supposed to already be precompiled for platforms I support!
// And we leak all the memory here intentionally. The engine itself leaks no memory though which is what
// really matters at the end of the day!

#include <string.h>

#include "public/string_builder.h"
#include "public/string_builder.c"

static bool game_provided     = false;

static bool given_name      = false;
static char game_name[1024] = {};

static bool dynamically_built = false;
static bool engine_no_build   = false;
static bool run_output        = false;

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

enum build_mode {
    BUILD_MODE_DEBUG,
    BUILD_MODE_RELEASE,
};
int build_mode = BUILD_MODE_DEBUG;

// Today I learned, strdup is actually not standard...
char* string_duplicate(char* original) {
    size_t string_length = strlen(original);
    char* new_string = malloc(string_length+1);
    new_string[string_length] = 0;
    for (size_t character_index = 0; character_index < string_length; ++character_index) {
        new_string[character_index] = original[character_index];
    }
    return new_string;
}

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

static bool string_match(char* a, char* b) {
    return strcmp(a, b) == 0;
}

#if defined(_WIN32)
#include "os_process_win32.c"
#elif defined(__linux__)
#include "os_process_linux.c"
#endif

static bool using_sdl2 =
#if defined(_WIN32)
    false;
#else
// on other platforms, they will probably just use SDL2.
// On windows this is optional.
    true;
#endif


static struct std_allocator _global_std_allocator = {};
// uses malloc and free
// consider moving into the main engine's public directory when I add a custom allocator.
// since this is actually kind of useful.

char* object_file_name(char* original_name) {

    size_t string_length = 0;
    // NOTE(jerry):
    // this is technically wrong... So whoops!
    // however it's not broken so I'm not complaining.
    // We just end up wasting a little bit of memory which is fine.
    // It's also really only safe because we read but never write to that location lol.
    {
        char* cursor = original_name;
        while (*(++cursor));
        string_length = cursor - original_name;
    }

    bool good_string = false;
    for (size_t index = 0; index < string_length && !good_string; ++index) {
        if (original_name[index] == '.') {
            good_string = true;
        }
    }

    if (!good_string) {
        return NULL;
    }

    size_t character_index = string_length - 1;
    for (; character_index != (size_t)(-1); --character_index) {
        if (original_name[character_index] == '.') {
            break;
        }
    }

    size_t new_string_size = character_index+2;
    char* name_with_object_extension = malloc(new_string_size);
    memset(name_with_object_extension, 0, new_string_size);

    strncpy(name_with_object_extension, original_name, character_index);
    name_with_object_extension[character_index++] = '.';
    name_with_object_extension[character_index++] = 'o';

    return name_with_object_extension;
}

static size_t source_file_list_count = 0;
static char*  source_file_list[65535];

// This will not do any checking, since it's intended to build, and the expected uses are
// pretty obvious, that I'm not going to attempt to check for any errors.

// Ideally, I would like this to just bootstrap future versions of itself, but on
// windows, this is kind of diffcult.
int main(int argument_count, char** argument_values) {
    _global_std_allocator = std_allocator_create();
    {
        // long options only for now.
        for (int argument_index = 1; argument_index < argument_count; ++argument_index) {
            char* current_argument = argument_values[argument_index];

            if (current_argument[0] == '-' && current_argument[1] == '-') { // program query
                char* long_argument_string = (current_argument + 2);

                if (string_match(long_argument_string, "help")) {
                    printf(
                        "build.exe will help build the blackiron engine with either a game or dynamic load mode.\n"
                        "While the engine can actually be built within one line, it's probably more convenient to just run this.\n"
                        "\n\n"
                        "build.exe +game %%1 %%...\n"
                        "\t %%1 is the entry point for the game, while other files are other translation units\n"
                        "\t for the game to build with. This game will be statically built.\n\n"
                        "build.exe +dynamic +game %%1 %%...\n"
                        "\t This will do the same as the above, however the game will be built as a dynamic link library\n"
                        "\t The engine will build separately!"
                        "build.exe\n"
                        "\t This will just build the engine by itself.\n\n"
                        "Adding +run to any of the above will also run the game with the engine.\n"
                        "So doing: build.exe +run +dynamic game.c, will build a game and run it.\n"
                        "The default name of the game output file will be \"game\" unless: +name is provided!\n"
                        "+game should ALWAYS be the LAST ARGUMENT provided if it is provided at all!\n"
                    );
                    return 0;
                } else {
                    printf("Unrecognized option --%s\n", long_argument_string);
                    return 0;
                }
            } else if (current_argument[0] == '+') { // program action
                char* action_name = current_argument+1;
                bool recognized = false;

                if (string_match(action_name, "name")) {
                    if (!given_name) {
                        char* found_game_name = argument_values[++argument_index];
                        strncpy(game_name, found_game_name, 1024);
                        given_name = true;
                    } else {
                        printf("Error, +name was provided multiple times?\n");
                        return 1;
                    }

                    recognized = true;
                }

                if (string_match(action_name, "run")) {
                    run_output = true;
                    recognized = true;
                }

                if (string_match(action_name, "noenginebuild")) {
                    engine_no_build = true;
                    recognized      = true;
                }

                if (string_match(action_name, "dynamic")) {
                    dynamically_built = true;
                    recognized = true;
                }

                if (string_match(action_name, "release")) {
                    build_mode = BUILD_MODE_RELEASE;
                    recognized = true;
                }

                if (string_match(action_name, "debug")) {
                    build_mode = BUILD_MODE_DEBUG;
                    recognized = true;
                }

                if (string_match(action_name, "sdl2")) {
                    if (operating_system != OPERATING_SYSTEM_WINDOWS) {
                        printf("On this platform there is no need to use SDL2. It is already built in!\n");
                        continue;
                    }
                    using_sdl2 = true;
                    recognized = true;
                }

                if (string_match(action_name, "game")) {
                    if (!given_name) {
                        strncpy(game_name, "game", 1024);
                    }

                    argument_index += 1;

                    while (argument_index < argument_count) {
                        if (argument_values[argument_index]) {
                            char* new_string = string_duplicate(argument_values[argument_index++]);
                            source_file_list[source_file_list_count++] = new_string;
                        }
                    }

                    game_provided = true;
                    recognized = true;
                }

                if (!recognized) {
                    printf("hmmm... I do not recognize +%s as a build action.\n", action_name);
                    return 1;
                }
            } else if (current_argument[0] == '-') {
                printf("This looks like an incomplete argument... Quitting!\n");
                return 1;
            }
        }
    }

    {
        // I will for now just assume my exact toolchain which is just a mingw64
        // toolchain on windows, and standard gcc on windows. This would also work with CLANG.
        string_builder build_command = { .allocator = &_global_std_allocator };

        // just build the engine as is.
        // add main files...

        if (operating_system == OPERATING_SYSTEM_WINDOWS) {
            string_builder_append_cstring(&build_command, "clang ");
            /* string_builder_append_cstring(&build_command, "gcc "); */
        } else {
            // I have no idea if my compiler toolchain is borked,
            // or whatever since SDL2 does not appear to build with clang on
            // my LUbuntu setup.
            //
            // 's fine though, since gcc on Linux is lightning fast.
            string_builder_append_cstring(&build_command, "gcc ");
        }

        // Entry point
        {
            if (using_sdl2) {
                string_builder_append_cstring(&build_command, "main_sdl2.c -D SDL2 ");
            } else {
                switch (operating_system) {
                    case OPERATING_SYSTEM_WINDOWS: {
                        string_builder_append_cstring(&build_command, "main_win32.c ");
                    } break;
                    case OPERATING_SYSTEM_LINUX: {
                        // this shouldn't happen.
                    } break;
                }
            }
        }

        // libraries / link flags
        {
            switch(operating_system) {
                case OPERATING_SYSTEM_WINDOWS: {
                    if (using_sdl2) {
                        string_builder_append_cstring(&build_command, "-lmingw32 -lSDL2main -lSDL2_mixer -lSDL2 ");
                    }

                    string_builder_append_cstring(&build_command, "-lglew32 -lxinput -lopengl32 -lgdi32 ");
                } break;
                case OPERATING_SYSTEM_LINUX: {
                    if (using_sdl2) {
                        string_builder_append_cstring(&build_command, "-lSDL2main -lSDL2_mixer -lSDL2 -lm ");
                    }

                    string_builder_append_cstring(&build_command, "-lGLEW -lGL ");
                } break;
            }

            string_builder_append_cstring(
                &build_command,
                "-I./. "
                "-std=c11"
                " -Wall"
                " -Wextra"
                " -Wno-unused-but-set-variable"
                " -Wno-unused-function"
                " -Wno-unused-parameter"
                " -Wno-unused-variable"
                " "
            );
        }

        // warnings and compilation flag modes.
        {
            {
                switch (build_mode) {
                    case BUILD_MODE_RELEASE: {
                        string_builder_append_cstring(&build_command, "-O2 ");
                    } break;
                    case BUILD_MODE_DEBUG: {
                        string_builder_append_cstring(&build_command, "-ggdb3 ");
                    } break;
                }
            }

            if (game_provided) {
                if (dynamically_built) {
                    static size_t object_file_list_count = 0;
                    static char*  object_file_list[65535];
                    {
                        // figure out the equivalent object file names.
                        for (size_t game_code_index = 0; game_code_index < source_file_list_count; ++game_code_index) {
                            char* object_name = object_file_name(source_file_list[game_code_index]);
                            if (object_name == NULL) {
                                fprintf(stderr, "Does not contain a file extension? Refusing to build!\n");
                                return 1;
                            } else {
                                object_file_list[object_file_list_count++] = object_name;
                            }
                        }
                    }

                    // okay... Let's build them all!
                    {
                        // admittedly... We could also build ALL the source files into one.
                        // but whatever.
                        char compilation_command_buffer[512];
                        // This is better than doing the string_builder over this part.
                        printf("okay... Build object files\n");
                        char* build_mode_string = "";

                        {
                            switch (build_mode) {
                                case BUILD_MODE_RELEASE: {
                                    build_mode_string = "-O2 ";
                                } break;
                                case BUILD_MODE_DEBUG: {
                                    build_mode_string = "-ggdb3 ";
                                } break;
                            }
                        }

                        for (size_t game_code_index = 0; game_code_index < source_file_list_count; ++game_code_index) {
                            snprintf(compilation_command_buffer, 512, "gcc %s -o %s -c -I./. %s", source_file_list[game_code_index], object_file_list[game_code_index], build_mode_string);
                            os_process_shell_start_and_run_synchronously(compilation_command_buffer);
                        }

                        {
                            string_builder linkage_builder = { .allocator = &_global_std_allocator };
                            string_builder_append_formatted_string(&linkage_builder, "gcc -shared -o %s%s ", game_name, shared_object_extension());

                            {
                                switch (build_mode) {
                                    case BUILD_MODE_RELEASE: {
                                        string_builder_append_cstring(&linkage_builder, "-O2 ");
                                    } break;
                                    case BUILD_MODE_DEBUG: {
                                        string_builder_append_cstring(&linkage_builder, "-ggdb3 ");
                                    } break;
                                }
                            }

                            for (size_t game_code_index = 0; game_code_index < source_file_list_count; ++game_code_index) {
                                string_builder_append_formatted_string(&linkage_builder, "%s ", object_file_list[game_code_index]);
                            }
                            printf("linkage command: %s\n", string_builder_construct(&linkage_builder));

                            os_process_shell_start_and_run_synchronously(string_builder_construct(&linkage_builder));
                        }
                    }
                } else {
                    // game source files.
                    for (size_t game_code_index = 0; game_code_index < source_file_list_count; ++game_code_index) {
                        string_builder_append_formatted_string(&build_command, "%s ", source_file_list[game_code_index]);
                    }

                    string_builder_append_cstring(&build_command, "-D BLACKIRON_STATIC_EXECUTABLE ");
                }
            }
        }

        // Still a bit of fine tuning required, but it does indeed work fine.
        if (game_provided && !dynamically_built) {
            string_builder_append_formatted_string(&build_command, "-o %s%s ", game_name, executable_extension());
        } else {
            string_builder_append_formatted_string(&build_command, "-o monkey%s ", executable_extension());
        }

        int build_result = -1;

        if (dynamically_built && engine_no_build) {
            // nothing
        } else {
            printf("build command: %s\n", string_builder_construct(&build_command));
            fflush(stdout);
            build_result = os_process_shell_start_and_run_synchronously(string_builder_construct(&build_command));
        }

        if (!run_output) {
            return build_result;
        } else {
            if (build_result != 0) {
                printf("There was an error in the build! Please fix this!\n");
                return -1;
            }

            string_builder run_command = { .allocator = &_global_std_allocator };

            if (operating_system == OPERATING_SYSTEM_LINUX) {
                string_builder_append_cstring(&run_command, "./");
            }

            if (!dynamically_built) {
                if (game_provided || given_name) {
                    string_builder_append_formatted_string(&run_command, "%s%s", game_name, executable_extension());
                } else {
                    string_builder_append_formatted_string(&run_command, "monkey%s", executable_extension());
                }
            } else {
                string_builder_append_formatted_string(&run_command, "monkey%s %s%s", executable_extension(), game_name, shared_object_extension());
            }

            fprintf(stderr, "run command: %s\n", string_builder_construct(&run_command));
            fflush(stdout);

            int result = os_process_shell_start_and_run_synchronously(string_builder_construct(&run_command));
            return result;
        }
    }
    return 0;
}
