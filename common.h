// I'm likely intending this to be used in a unity build situation
// this is for common code, or anything too small to deserve it's own file.
// so this might get quite chunky with random stuff.
// or containers or something like that.
#include <stdio.h>
#include <stdlib.h>

#include "public/common.h"

#define FRAMERATE_SAMPLER_TIME_SAMPLES (64)
static float _sampled_times[FRAMERATE_SAMPLER_TIME_SAMPLES] = {};
static uint8_t _sample_index                                = 0;
static uint8_t _collected_samples                           = 0;
static void framerate_sampler_update(float frametime) {
    _sampled_times[_sample_index++] = frametime;
    _sample_index &= (FRAMERATE_SAMPLER_TIME_SAMPLES - 1);

    if (_collected_samples < FRAMERATE_SAMPLER_TIME_SAMPLES) {
        _collected_samples++;
    }
}

float framerate_sampler_average_frametime(void) {
    if (_collected_samples <= 0) {
        return 0.0;
    }

    float average = 0.0;

    for (size_t sample_index = 0; sample_index < _collected_samples; ++sample_index) {
        average += _sampled_times[sample_index];
    }

    average /= _collected_samples;

    return average;
}

float framerate_sampler_average_framerate(void) {
    return (1.0f) / (framerate_sampler_average_frametime());
}


// tracked malloc I really only care about tracking my own memory
// allocations (this also excludes stb which although can replace
// allocators, is not worth tracking, since it's only for assets so
// I'm less iffy about that.)  virtual memory will be considered
// separate from malloc if I use it.  But I'm probably just going to
// malloc big chunks.

// TODO(jerry): I should track the file and line they come from.
// I'm only really concerned about doing this from engine code though
// I don't technically care if you leak memory in game code. That's
// not really my responsibility... (though I should provide a tracked alloc in that case.)

// All this is going to do is just track usage.
// It's up to smaller memory managers to figure out what their used for
// so I'm not tagging them here.
static size_t _memory_allocated_total = 0;
struct memory_allocation_header_tracking {
    size_t amount;
};
// this is a guesstimate without accounting for alignment.
// the amount does not include header amount.
// header is added onto amount, which is a little unusual to think about
// however this is going to be ifdefed away in release mode
void* system_memory_allocate(size_t amount) {
    amount += sizeof(struct memory_allocation_header_tracking);
    void* pointer = malloc(amount);
    memset(pointer, 0, amount);

    if (pointer) {
        _memory_allocated_total += amount;
        struct memory_allocation_header_tracking* header = pointer;
        header->amount = amount;
        return pointer + sizeof(struct memory_allocation_header_tracking);
    }

    return NULL;
}

void* system_memory_reallocate(void* base, size_t amount) {
    // I assume base came from one system_memory_allocate
    // so the real pointer is from there.
    base -= sizeof(struct memory_allocation_header_tracking);
    {
        struct memory_allocation_header_tracking* header = base;
        _memory_allocated_total -= header->amount;
    }

    amount += sizeof(struct memory_allocation_header_tracking);

    void* realloced_data = realloc(base, amount);

    if (realloced_data) {
        _memory_allocated_total += amount;

        struct memory_allocation_header_tracking* header = realloced_data;
        header->amount = amount;

        return realloced_data + sizeof(struct memory_allocation_header_tracking);
    }

    return NULL;
}

void system_memory_deallocate(void* pointer) {
    struct memory_allocation_header_tracking* header = (struct memory_allocation_header_tracking*)pointer - 1;
    _memory_allocated_total -= header->amount;
    free(header);
}

static size_t memory_allocated_total(void) {
    return _memory_allocated_total;
}

static void _system_memory_leak_check(void) {
    if (_memory_allocated_total != 0) {
        fprintf(stderr, "There's probably a memory leak somewhere! I need to track these more! (%lu bytes leaked?)\n", _memory_allocated_total);
    } else {
        fprintf(stderr, "No memory leaks! Good programming guys!\n");
    }
}

#if __EMSCRIPTEN__
// Emscripten does not support x86 intrinsics, and their LLVM shits
// itself, so this has to be special cased.
static uint64_t read_timestamp_counter(void) {
    return 0;
}
#else
#if Compiler == gnu_c || Compiler == clang
#include <x86intrin.h>
static uint64_t read_timestamp_counter(void) {
    return __rdtsc();
}
#elif Compiler == visual_c
#include <intrin.h>
#pragma intrinsic(__rdtsc)
static uint64_t read_timestamp_counter(void) {
    return __rdtsc();
}
#else
static uint64_t read_timestamp_counter(void) {
    return 0;
}
#endif
#endif
