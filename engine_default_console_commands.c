// TODO(jerry): variable setting.
Define_Console_Variable(engine_version, STRING);

Define_Console_Command(exit) {
    application_quit = true;
}

Define_Console_Command(clear) {
    console_clear();
}

Define_Console_Command(gfx_mem_usage) {
    struct graphics_context* graphics_context = &application_graphics_context;
    {
        size_t c = graphics_context->memory_capacity;
        console_printf("%llu B (%llu KB), %llu MB capacity\n", c, c / 1024, c / (1024*1024));
        size_t u = graphics_context->memory_used + graphics_context->memory_used_top;
        if (u > c) {
            size_t o = u - c;
            console_printf("FATAL: You have overrun memory by: %llu B (%llu KB) %llu MB!\n", o, o/1024, o/(1024*1024));
        }
        console_printf("%llu B, %llu KB, %llu MB\n", u, u / 1024, u / (1024*1024));
        size_t l = c - u;
        console_printf("%llu B, %llu KB, %llu MB left\n", l, l / 1024, l / (1024*1024));
    }
}

Define_Console_Command(echo_var) {
    if (argument_count != 1) {
        console_printf("echo variable only requires the variable to echo!\n");
        return;
    } else {
        // do type checking on the name lol.
        printf("%s\n", parameters[0].string);
        struct console_system_variable* variable = console_system_find_variable(parameters[0].string);
        if (variable != &_global_console_system.variable_list.null) {
            {
                console_printf("%s = ", variable->name);

                // A switch case would actually work... But this is more explicit to the intent that
                // this is actually a bitfield.
                if (variable->type & CONSOLE_VARIABLE_TYPE_NUMBER) {
                    if (variable->type & CONSOLE_VARIABLE_TYPE_NUMBER_REAL) {
                        console_printf("%f", variable->real);
                    } else if (variable->type & CONSOLE_VARIABLE_TYPE_NUMBER_INTEGER) {
                        console_printf("%d", variable->integer);
                    }
                } else if (variable->type & CONSOLE_VARIABLE_TYPE_STRING) {
                    console_printf("%.*s", CONSOLE_SYSTEM_VARIABLE_STRING_SIZE, variable->string);
                } else if (variable->type & CONSOLE_VARIABLE_TYPE_BOOLEAN) {
                    if (variable->boolean) {
                        console_printf("true");
                    } else {
                        console_printf("false");
                    }
                }
                console_printf("\n");
            }
        } else {
            console_printf("There is no variable by the name of \"%s\"!\n", parameters[0].string);
        }
    }
}

Define_Console_Command(listcommands) {
    if (argument_count > 1) {
        console_printf("list all commands takes no arguments!\n");
        return;
    }

    console_printf("=========== COMMAND LIST ==========\n");
    for (console_system_command_iterator commands = console_system_begin_iterating_commands();
         !console_system_command_iterator_finished(&commands);
         console_system_command_iterator_advance(&commands)) {
        console_printf("cmd: %s\n", commands.current->name);
    }
    console_printf("=========== END COMMAND LIST ==========\n");
}

Define_Console_Command(set_fullscreen) {
    if (argument_count == 1) {
        if (parameters[0].type & CONSOLE_VARIABLE_TYPE_NUMBER) {
            if (!(parameters[0].type & CONSOLE_VARIABLE_TYPE_NUMBER_INTEGER)) {
                console_printf("set_fullscreen should only be getting an integer value!\n");
                return;
            }

            _global_system_api.set_window_fullscreen(parameters[0].integer);
        } else {
            console_printf("set_fullscreen needs to receive a number argument!\n");
            return;
        }
    } else {
        console_printf("set_fullscreen only needs one argument!\n");
    }
}

Define_Console_Command(set_resolution) {
    if (argument_count == 2) {
        if (!(parameters[0].type & CONSOLE_VARIABLE_TYPE_NUMBER)) {
            goto non_number_error;
        } else {
            if (!(parameters[0].type & CONSOLE_VARIABLE_TYPE_NUMBER_INTEGER)) {
                goto non_integer_error;
            }
        }

        if (!(parameters[1].type & CONSOLE_VARIABLE_TYPE_NUMBER)) {
            goto non_number_error;
        } else {
            if (!(parameters[1].type & CONSOLE_VARIABLE_TYPE_NUMBER_INTEGER)) {
                goto non_integer_error;
            }
        }

        _global_system_api.set_window_resolution(parameters[0].integer, parameters[1].integer);
        return;
    } else {
        console_printf("set_resolution requires two integer arguments!\n");

        return;
    }

non_number_error:
    console_printf("One of the parameters for set_resolution was not a number!\n");
    return;
non_integer_error:
    console_printf("One of the parameters for set_resolution was not an integer!\n");
    return;
}


Define_Console_Command(reload_all_fonts) {
    if (argument_count > 0) {
        console_printf("reload_all_fonts has no arguments!\n");
        return;
    }

    graphics_context_reload_all_fonts(&application_graphics_context);
}

Define_Console_Command(reload_all_textures) {
    if (argument_count > 0) {
        console_printf("reload_all_textures has no arguments!\n");
        return;
    }

    graphics_context_reload_all_textures(&application_graphics_context);
}

Define_Console_Command(reload_all_shaders) {
    if (argument_count > 0) {
        console_printf("reload_all_shaders has no arguments!\n");
        return;
    }

    graphics_context_reload_all_shaders(&application_graphics_context);
}

Define_Console_Command(reload_all_resources) {
    if (argument_count > 0) {
        console_printf("reload_all_resources has no arguments!\n");
        return;
    }

    graphics_context_reload_all_resources(&application_graphics_context);
}

Define_Console_Command(test_matcher) {
    /* void _console_determine_best_completion_match(char*); */
    /* _console_determine_best_completion_match("reload"); */
    /* _console_determine_best_completion_match("all"); */
    /* _console_determine_best_completion_match("all_shaders"); */
    /* _console_determine_best_completion_match("set_fullscreen"); */
    /* _console_determine_best_completion_match("set"); */
}

static void _register_default_commands(void) {
    {
#ifdef SDL2
        strncpy(convar_engine_version.string, "SDL2 0.0.0v", CONSOLE_SYSTEM_VARIABLE_STRING_SIZE);
#else
        strncpy(convar_engine_version.string, "0.0.0v", CONSOLE_SYSTEM_VARIABLE_STRING_SIZE);
#endif

#if __EMSCRIPTEN__
        strncpy(convar_engine_version.string, "Emscripten WASM 0.0.0v", CONSOLE_SYSTEM_VARIABLE_STRING_SIZE);
#endif

        console_system_register_variable(&convar_engine_version);

        console_system_register_command(&cmd_echo_var);

        {
            console_system_register_command(&cmd_test_matcher);
        }

        {
            console_system_register_command(&cmd_gfx_mem_usage);
            console_system_register_command(&cmd_listcommands);
            console_system_register_command(&cmd_exit);
            console_system_register_command(&cmd_clear);
        }

        {
            console_system_register_command(&cmd_set_resolution);
            console_system_register_command(&cmd_set_fullscreen);
        }

        {
            console_system_register_command(&cmd_reload_all_resources);
            console_system_register_command(&cmd_reload_all_textures);
            console_system_register_command(&cmd_reload_all_shaders);
            console_system_register_command(&cmd_reload_all_fonts);
        }
    }
}
