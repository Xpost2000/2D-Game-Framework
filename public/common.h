#ifndef PUBLIC_COMMON_H
#define PUBLIC_COMMON_H

// TODO(jerry):
// introduce platform wrappers around these things to avoid issues
// Probably introduce them as part of the system API if possible.
// (system->format_string)  - snprintf
// (system->format)         - printf
// (system->format_log)     - printf with log level or something
// (system->console_format) - Engine console printf
#include <stdio.h> // snprintf is cool though.
/* #include <stdlib.h> // string.h is mostly cool though. I'll have to reimplement those though... As I don't want all the symbols to spill over. */

#include <stdbool.h>

#include <stdint.h>
#include <stddef.h>

#include <assert.h>

#include <math.h>
#include <time.h>

// It is legal to take the address of anything and read from any pointer.
// Just don't write to them!
// Interpret a null pointer as a pointer of the type I want, and taking the address of
// any members and subtracting them by the base pointer will always give me the absolute offset
// in bytes. (provided I cast into a byte pointer.)

// MinGW has a builtin offsetof
// actually probably all compilers do? But just in case I guess.
#ifdef __GNUC__
#else
#define offsetof(type, member) (char*)&(((type*)(0))->member) - (char*)((type*)(0))
#endif

#define define_new_handle_type(underlying_type, name)   \
    typedef struct name {                               \
        underlying_type id;                             \
    } name                                    

#ifdef _MSC_VER
#define Force_inline __forceinline
#else
#define Force_inline __attribute__((always_inline))
#endif

// NOTE(jerry):
// man I've got some really inconsistent naming conventions for macros
// I should probably just Pascal_Case them
// to differentiate them from normal things.
// or like
// Title_case_like_this.

#define BIT(x)   (1LL << x)
#define BIT32(x) (1 << x)

// I really want to be able to use typeof...
// but typeof is only implemented by GNU extension compilers...
// means I can't use compound expressions either...

// Why does GNU C have so many useful extensions?? And why does Microsoft not do it but
// Apple would?

// I would almost like to consider just dropping MSVC support, if it weren't so ubiquitous on
// Windows... And to be fair it wouldn't be standard C anymore... Which is kind of one of the things
// I want it to be. Ah whatever, this isn't that much more work. It just makes the code a little less
// nicer to write (although it doesn't really save me more than a couple seconds anyways.)
#define Swap(type, a, b)                        \
    do {                                        \
        type temporary = a;                     \
        a = b;                                  \
        b = temporary;                          \
    } while (0);

#define Kibibyte(x) (x * 1024)
#define Mebibyte(x) (x * 1024 * 1024)
#define Gibibyte(x) (x * 1024 * 1024 * 1024)

#define assertion(x) (assert(x))

// NOTE(jerry): totally not evil.

#if Compiler == gnu_c || Compiler == clang
#pragma GCC diagnostic ignored "-Wreturn-type"
#pragma GCC diagnostic push
#endif

// While this is under public... I don't think any public user has a reason to use this honestly.
#define illegal_instruction() do { __builtin_trap(); } while(0)
#define unreachable_code()                                              \
    do {                                                                \
        fprintf(stderr, "Unreachable code: %s:%d:%s\n", __FILE__, __LINE__, __func__); \
        illegal_instruction();                                          \
        return;                                                         \
    } while (0)
#define unimplemented(note)                                             \
    do {                                                                \
        fprintf(stderr, "Unimplemented code: %s:%d:%s\n", __FILE__, __LINE__, note); \
        return;                                                         \
    } while (0)

#if Compiler == gnu_c || Compiler == clang
#pragma GCC diagnostic pop
#endif

#define _intentionally_unused(x) ((void)(x))

// TODO(jerry): string utility stuff.
static bool is_whitespace_character(char character) {
    return (character == '\n' ||
            character == ' '  ||
            character == '\r' ||
            character == '\t');
}

static bool is_lowercase_character(char character) {
    return (character >= 'a') && (character <= 'z');
}

static bool is_uppercase_character(char character) {
    return (character >= 'A') && (character <= 'Z');
}

static bool is_alphabetic_character(char character) {
    return (is_lowercase_character(character) ||
            is_uppercase_character(character));
}

static bool is_numeric_character(char character) {
    return (character >= '0') && (character <= '9');
}

static bool is_ascii_character(char character) {
    return (character >= 0 && character <= 127);
}

static bool is_human_readable_ascii_character(char character) {
    return (character >= 32 && character <= 127);
}

static uint64_t fnv1_hash(void* data, size_t data_length) {
    char* bytes = data;
    uint64_t fnv_prime   = 0x100000001b3;
    uint64_t fnv_basis   = 0xcbf29ce484222325;

    uint64_t hash_result = 0xcbf29ce484222325;

    for (size_t byte_index = 0; byte_index < data_length; ++byte_index) {
        hash_result *= fnv_prime;
        hash_result ^= bytes[byte_index];
    }

    return hash_result;
}
static uint64_t fnv1a_hash(void* data, size_t data_length) {
    char* bytes = data;
    uint64_t fnv_prime   = 0x100000001b3;
    uint64_t fnv_basis   = 0xcbf29ce484222325;

    uint64_t hash_result = 0xcbf29ce484222325;

    for (size_t byte_index = 0; byte_index < data_length; ++byte_index) {
        hash_result ^= bytes[byte_index];
        hash_result *= fnv_prime;
    }

    return hash_result;
}

#include <stdarg.h>
#define IMMEDIATE_FORMAT_TEXT_BUFFER_SIZE (256)
#define IMMEDIATE_FORMAT_TEXT_CYCLIC_BUFFER_LENGTH (8)
static char* immediate_format_text(const char* format_string, ...) {
    static char    immediate_format_buffers[IMMEDIATE_FORMAT_TEXT_CYCLIC_BUFFER_LENGTH][IMMEDIATE_FORMAT_TEXT_BUFFER_SIZE] = {};
    static uint8_t immediate_current_buffer_index = 0;

    char* current_buffer = immediate_format_buffers[immediate_current_buffer_index++];
    immediate_current_buffer_index &= (IMMEDIATE_FORMAT_TEXT_CYCLIC_BUFFER_LENGTH - 1);

    va_list variadic_arguments;

    va_start(variadic_arguments, format_string);
    vsnprintf(current_buffer, IMMEDIATE_FORMAT_TEXT_BUFFER_SIZE, format_string, variadic_arguments);
    va_end(variadic_arguments);

    return current_buffer;
}

// undef for math.h
#undef min
#undef max

// I do not template these, I really do type these out by hand :/
static float sign_float(float x) {
    if (x > 1) {
        return 1.0;
    } else if (x == 0) {
        return 0.0; 
    } else {
        return -1.0;
    }
}
static float maximum_float(float a, float b) {
    if (a > b) {
        return a;
    } else {
        return b;
    }
}
static float minimum_float(float a, float b) {
    if (a < b) {
        return a;
    } else {
        return b;
    }
}
static float clamp_float(float x, float minimum, float maximum) {
    if (x < minimum) {
        return minimum;
    } else if (x > maximum) {
        return maximum;
    }

    return x;
}

#ifndef PI
#define PI (3.14159265358979323846)
#endif
static float degree_to_radians(float degrees) {
    return (degrees * (PI / 180.0f));
}

static float radians_to_degrees(float radians) {
    return (radians * (180.0f / PI));
}

static uint32_t encode_rgba_from_float(float r, float g, float b, float a) {
    uint8_t red_byte   = clamp_float(r, 0.0, 1.0) * 255.0f;
    uint8_t green_byte = clamp_float(g, 0.0, 1.0) * 255.0f;
    uint8_t blue_byte  = clamp_float(b, 0.0, 1.0) * 255.0f;
    uint8_t alpha_byte = clamp_float(a, 0.0, 1.0) * 255.0f;

    return (red_byte << 24 | green_byte << 16 | blue_byte << 8 | alpha_byte);
}
static void decode_rgba_from_uint32(uint32_t encoded_rgba, float* rgba) {
    rgba[0] = (float)((encoded_rgba & 0xFF000000) >> 24) / 255.0f;
    rgba[1] = (float)((encoded_rgba & 0x00FF0000) >> 16) / 255.0f;
    rgba[2] = (float)((encoded_rgba & 0x0000FF00) >> 8)  / 255.0f;
    rgba[3] = (float)((encoded_rgba & 0x000000FF))       / 255.0f;
}

static void decode_rgba_from_uint32_unnormalized(uint32_t encoded_rgba, uint8_t* rgba) {
    rgba[0] = ((encoded_rgba & 0xFF000000) >> 24);
    rgba[1] = ((encoded_rgba & 0x00FF0000) >> 16);
    rgba[2] = ((encoded_rgba & 0x0000FF00) >> 8);
    rgba[3] = ((encoded_rgba & 0x000000FF));
}

static void decode_rgba_from_uint32_safe(uint32_t encoded_rgba, float* rgba, size_t length) {
    if (length > 4) {
        length = 4;
    }

    static uint32_t bitmask[4] = {
        0xFF000000,
        0x00FF0000,
        0x0000FF00,
        0x000000FF,
    };

    static uint8_t bitshift[4] = {
        24,
        16,
        8,
        0
    };

    for (size_t float_index = 0; float_index < length; ++float_index) {
        rgba[float_index] = (float)((encoded_rgba & bitmask[float_index]) >> bitshift[float_index]) / 255.0f;
    }
}

static int64_t next_power_of_two(int64_t n) {
    int64_t exponent = 0;

    while ((1 << exponent) < n) {
        exponent++;
    }
    
    return 1 << exponent;
}

static int64_t nearest_power_of_two(int64_t n) {
    // urg..
    int64_t npot     = next_power_of_two(n);
    int64_t ppot     = npot >> 1;

    if ((npot - n) < (n - ppot)) {
        return npot;
    } else {
        return ppot;
    }
}

static float inverse_linear_interpolate_float(float minimum, float maximum, float input_value) {
    float value_range = (minimum     - maximum);
    float value_delta = (input_value - maximum);

    return value_delta / value_range;
}

static float linear_interpolate_float(float minimum, float maximum, float t) {
    if (t <= 0.0f) {
        t = 0.0f;
    }

    if (t >= 1.0f) {
        t = 1.0f;
    }

    return (minimum) * (t - 1.0) + (maximum * t);
}

static float linear_remap_float(float minimum_output_value, float maximum_output_value, float minimum_input_value, float maximum_input_value, float input_value) {
    float input_value_time_value;
    {
        input_value_time_value = (input_value - minimum_input_value) / (maximum_input_value - minimum_input_value);
    }

    float remapped_value;
    {
        remapped_value = (minimum_output_value) * (input_value_time_value - 1.0) + maximum_output_value * (input_value_time_value);
    }

    return remapped_value;
}

// http://gizma.com/easing/
static float cubic_ease_in_interpolate_float(float minimum, float maximum, float t) {
    return (maximum - minimum) * (t * t * t) + minimum;
}

static float cubic_ease_out_interpolate_float(float minimum, float maximum, float t) {
    t -= 1;
    return (maximum - minimum) * ((t * t * t) + 1.0) + minimum;
}

static float quadratic_ease_in_interpolate_float(float minimum, float maximum, float t) {
    return (maximum - minimum) * (t * t) + minimum;
}

static float quadratic_ease_out_interpolate_float(float minimum, float maximum, float t) {
    return -(maximum - minimum) * ((t * t) - 2.0) + minimum;
}

static size_t cstring_length(char* cstring) {
    char* cursor = cstring;

    while (*(cursor)) {
        cursor++;
    }

    return (cursor - cstring);
}

// cool ass defer macro
// from here
// https://accu.org/conf-docs/PDFs_2021/luca_sass_modern_c_and_what_we_can_learn_from_it.pdf
#define __defer_concat(a, b) a##_DEFER_##b
#define __defer_var(a) __defer_concat(a, __LINE__)
#define Defer(start, end) for (                 \
        int __defer_var(_i_) = (start, 0);      \
        !__defer_var(_i_);                      \
        (__defer_var(_i_) += 1), end)

#endif
