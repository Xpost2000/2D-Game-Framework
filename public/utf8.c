// This is most definitely not a fully compliant UTF8 decoder,
// but it's good enough I guess.
static const uint8_t utf8_sequence_len[0x100] =
{
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* 0x00-0x0F */
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* 0x10-0x1F */
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* 0x20-0x2F */
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* 0x30-0x3F */
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* 0x40-0x4F */
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* 0x50-0x5F */
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* 0x60-0x6F */
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* 0x70-0x7F */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x80-0x8F */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0x90-0x9F */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0xA0-0xAF */
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0xB0-0xBF */
    0,0,2,2,2,2,2,2,2,2,2,2,2,2,2,2, /* 0xC0-0xCF */
    2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, /* 0xD0-0xDF */
    3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3, /* 0xE0-0xEF */
    4,4,4,4,4,0,0,0,0,0,0,0,0,0,0,0, /* 0xF0-0xFF */
};

// There are more specific rules but this is a good litmus test?
static size_t utf8_required_bytes(uint8_t leading_byte) {
    struct {
        int mask;
        size_t count;
    } table[] = {
        {0x0,  1},
        {0xC2, 2},
        {0xE0, 3},
        {0xF0, 4},
    };

    for (size_t testing_index = sizeof(table)/sizeof(*table)-1; testing_index != (size_t)(-1); testing_index--) {
        if ((leading_byte & table[testing_index].mask) == table[testing_index].mask) {
            return table[testing_index].count;
        }
    }

    return 0;
}

struct utf8_decode_result utf8_decode_single_codepoint(char* buffer) {
    struct utf8_decode_result decoded_packet = (struct utf8_decode_result){
        .codepoint     = 0,
        .decoded_bytes = 0,
        .error         = false,
    };

    size_t byte_length = utf8_required_bytes((uint8_t)buffer[0]);

    decoded_packet.decoded_bytes = byte_length;
    
    // NOTE(jerry):
    // This... May depend on endianness. I'm kind of hoping it doesn't.
    // but I can always force little endianness later
    uint8_t byte0 = buffer[0];
    uint8_t byte1 = buffer[1];
    uint8_t byte2 = buffer[2];
    uint8_t byte3 = buffer[3];

    switch (byte_length) {
        case 1: {
            decoded_packet.codepoint = byte0;
        } break;
        case 2: {
            // BYTE 1        |      BYTE 2
            // 110xxxxx            10xxxxxx
      // MASK  00011111            00111111
      // HEX   0x1F                0x3F
            decoded_packet.codepoint =
                (byte0 & 0x1F /* 0b00011111 */) << 6 |
                (byte1 & 0x3F /* 0b00111111 */);
        } break;
        case 3: {
            // BYTE 1        |      BYTE 2     |       BYTE 3
            // 110xxxxx            10xxxxxx         10xxxxxx
      // MASK  00011111            00111111         00111111
      // HEX   0x1F                0x3F
            decoded_packet.codepoint =
                (byte0 & 0x1F /* 0b00011111 */) << 12 |
                (byte1 & 0x3F /* 0b00111111 */) << 6 |
                (byte2 & 0x3F /* 0b00111111 */);
        } break;
        case 4: {
            // BYTE 1        |      BYTE 2     |       BYTE 3    |  BYTE 4
            // 110xxxxx            10xxxxxx         10xxxxxx       10xxxxxx
      // MASK  00011111            00111111         00111111       00111111
      // HEX   0x1F                0x3F
            decoded_packet.codepoint =
                (byte0 & 0x1F /* 0b00011111*/) << 18 |
                (byte1 & 0x3F /* 0b00111111*/) << 12 |
                (byte2 & 0x3F /* 0b00111111*/) << 6  |
                (byte3 & 0x3F /* 0b00111111*/);
        } break;
        default: {
            decoded_packet.error = true;
        } break;
    }

    return decoded_packet;
}

uint32_t decode_utf8_iterator_advance(decode_utf8_iterator* iterator);
decode_utf8_iterator decode_utf8_from(char* buffer, size_t buffer_length) {
    decode_utf8_iterator iterator = (decode_utf8_iterator) {
        .buffer        = buffer,
        .buffer_length = buffer_length,
        .decoded_count = 0,
        .codepoint     = 0,
    };

    // This is done because to get the correct initial expected state, which
    // is the first character.
    decode_utf8_iterator_advance(&iterator);
    return iterator;
}

// NOTE(jerry):
// This may be a little iffy, I need to check a bit later on how specifically
// I consider the buffer slice here. As I've only tested this on C strings.
decode_utf8_iterator decode_utf8_from_cstring(char* buffer) {
    size_t string_length = 0;
    char* cursor = buffer;

    while (*(cursor++)) {}
    string_length = cursor - buffer;

    return decode_utf8_from(buffer, string_length);
}

bool decode_utf8_iterator_valid(decode_utf8_iterator* iterator) {
    if (iterator->decoded_count < iterator->buffer_length) {
        return true;
    }

    return false;
}

uint32_t decode_utf8_iterator_advance(decode_utf8_iterator* iterator) {
    char*  buffer        = iterator->buffer;
    size_t decoded_count = iterator->decoded_count;

    struct utf8_decode_result decoded_packet = utf8_decode_single_codepoint(&buffer[decoded_count]);

    if (decoded_packet.error) {
        return 0;
    } else {
        iterator->decoded_count += decoded_packet.decoded_bytes;
        iterator->codepoint      = decoded_packet.codepoint;
    }

    return iterator->codepoint;
}
