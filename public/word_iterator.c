word_iterator word_iterator_begin_iterating_from(char* buffer, size_t buffer_length) {
    word_iterator result =  (word_iterator) {
        .buffer        = buffer,
        .buffer_length = buffer_length,
        .read_cursor   = 0,
        .done          = false
    };

    // Same reasoning as the UTF8 iterator.
    word_iterator_advance(&result);
    return result;
}

word_iterator word_iterator_begin_iterating_from_cstring(char* buffer) {
    size_t string_length = 0;
    char* cursor = buffer;

    while (*(++cursor)) {}
    string_length = cursor - buffer;

    return word_iterator_begin_iterating_from(buffer, string_length);
}

bool word_iterator_finished(word_iterator* iterator) {
    // Technically a index comparison should work, but the iterator advancer... Is technically
    // what actually determines if we're done or not lol.
    return iterator->done;
}

void word_iterator_advance(word_iterator* iterator) {
    // This is done here to make sure the for loop will work as expected.
    if (iterator->read_cursor > iterator->buffer_length) {
        iterator->done = true;
        return;
    }

    char* start_of_word = iterator->buffer + iterator->read_cursor;
    char* end_of_word   = start_of_word;

    bool found_string = false;

    while (iterator->read_cursor < iterator->buffer_length) {
        char current = iterator->buffer[iterator->read_cursor];

        if (!found_string && current == '\"') {
            found_string = true;
            start_of_word++;
            current = iterator->buffer[++iterator->read_cursor];
        }

        if (!found_string) {
            if (is_whitespace_character(current)) {
                break;
            }
        } else {
            if (current == '\"') {
                break;
            }
        }

        iterator->read_cursor++;
    }

    // This actually almost looks like a mistake, because it looks like a buffer overrun.
    // I mean, it actually IS a buffer overrun, but this is still fine since I don't read or write
    // and it does give an honest length reading lol.

    // The buffer overrun never happens within the loop as far as I'm aware so there's no issue.
    end_of_word = iterator->buffer + iterator->read_cursor++;

    size_t word_length    = end_of_word - start_of_word;
    iterator->word        = start_of_word;
    iterator->word_length = word_length;

    // NOTE(jerry):
    // This is an incredibly lazy solution.
    // To solve accidently tokenizing whitespace.

    // This replicates the same solution I would have done, which is just eat whitespace, but this is way
    // shorter... Also I hope this is actually tail recursive like I think it is.
    if (end_of_word != start_of_word) {
        return;
    }

    word_iterator_advance(iterator);   
}

size_t word_iterator_length(word_iterator* iterator) {
    word_iterator clone = *iterator;
    size_t result = 0;
    while (!word_iterator_finished(&clone)) {
        result++;
        word_iterator_advance(&clone);
    }
    return result;
}
