#ifndef UTF8_H
#define UTF8_H

// This is only for decoding primarily,
// I don't believe I have an interest in encoding right now since
// UTF8 is probably the sanest encoding format.

struct utf8_decode_result {
    uint32_t codepoint;
    size_t   decoded_bytes;
    bool     error;
};

// So I guess a major rule is:
//
// Handle types and iterator types are typedefed
// basically anything that isn't really supposed to be a real "structure"
// or is otherwise a very simple wrapper for menial code should be typedefed.
//
typedef struct decode_utf8_iterator {
    char* buffer;
    size_t buffer_length;
    size_t decoded_count;
    uint32_t codepoint;
} decode_utf8_iterator;
decode_utf8_iterator decode_utf8_from(char* buffer, size_t buffer_length);
decode_utf8_iterator decode_utf8_from_cstring(char* buffer);
bool                 decode_utf8_iterator_valid(decode_utf8_iterator* iterator);
uint32_t             decode_utf8_iterator_advance(decode_utf8_iterator* iterator);

// NOTE(jerry):
// This does not do any safety checking on the buffer. It will try to blindly convert codepoints.
// That's up to the caller to handle
// TODO(jerry):
// Well, I should error out if we cannot finish decoding...
// TODO(jerry):
// should provide iterator interface to simplify the text drawing code that relies on
// utf8 decoding
struct utf8_decode_result utf8_decode_single_codepoint(char* buffer);
/* struct utf8_encode_result utf8_encode_utf32/unicode_string(uint32_t* string, size_t length) */

#endif
