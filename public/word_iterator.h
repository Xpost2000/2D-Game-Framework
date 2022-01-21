#ifndef WORD_ITERATOR_H
#define WORD_ITERATOR_H

// a word iterator that tokenizes space delimited strings
// and double quoted strings

typedef struct word_iterator {
    char* buffer;
    size_t buffer_length;
    size_t read_cursor;

    bool done;

    char* word;
    size_t word_length;
} word_iterator;

word_iterator word_iterator_begin_iterating_from_cstring(char* buffer);
word_iterator word_iterator_begin_iterating_from(char* buffer, size_t buffer_length);
void          word_iterator_advance(word_iterator* iterator);
bool          word_iterator_finished(word_iterator* iterator);

size_t word_iterator_length(word_iterator* iterator);
#endif
