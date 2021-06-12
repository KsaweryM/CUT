#ifndef __STRING_BUFFER_H__
#define __STRING_BUFFER_H__

#include <stddef.h>
/*
*   String_buffer is a buffer used to transfer string between threads.
*/
typedef struct string_buffer string_buffer;

/*
*   A method to create string_buffer.
*   length: how many strings string_buffer can store.
*   return: pointer to created string_buffer.
*/
string_buffer* string_buffer_create(const size_t words_count, const size_t word_length);

/*
*   A method to destroy string_buffer.
*/
void string_buffer_destroy(string_buffer* buffer);

/*
*   A method to insert value into string_buffer.
*   Calling this method blocks a thread as long as string_buffer is full.
*/
size_t string_buffer_write(string_buffer* buffer, const char data[]);

/*
*   Save oldest value from string_buffer and remove it from string_buffer.
*   Calling this method blocks a thread as long as buffer is empty.
*/
size_t string_buffer_read(string_buffer* buffer, char* data, size_t available_space);

#endif