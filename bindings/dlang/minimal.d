import host_api;

struct game_state {
    hello_world_font_handle arial_font = {};
};
__gshared game_state _global_state;

__gshared hello_world_api* graphics;

extern(C) void initialize(host_state* state, int argc, char** argv) {
    graphics = state.graphics;
    _global_state.arial_font = graphics.load_font_from_file("resources/console/LiberationMono-Bold.ttf", 36);
}

extern(C) void frame(host_state* state, float delta_time) {
    scope(exit) state.elapsed_time += delta_time;

    graphics.clear_buffer(1.0, 1.0, 1.0, 1.0);
    graphics.set_virtual_resolution();

    graphics.with_drawing(
        function() {
            for (int i = 0; i < 10; ++i) {
                graphics.draw_text(_global_state.arial_font,
                                   "Hello World! From DLang!",
                                   30+3, 100 + i *36+3,
                                   1.0, 0.0, 0.0, 0.0, 1.0);
                graphics.draw_text(_global_state.arial_font,
                                   "Hello World! From DLang!",
                                   30, 100 + i *36,
                                   1.0, 1.0, 0.0, 0.0, 1.0);
            }
        }
    );
}

extern(C) host_api client_get_host_api() {
    host_api result;
    result.initialize = &initialize;
    result.frame = &frame;
    return result;
}
