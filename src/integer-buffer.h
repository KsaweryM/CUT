#ifndef __INTEGER_BUFFER_H__
#define __INTEGER_BUFFER_H__

#include <stddef.h>

/*
*   Integer_buffer is a buffer used to transfer integers between threads
*/
typedef struct integer_buffer integer_buffer;

/*
*   A method to create integer_buffer.
*   length: how many integers integer_buffer can store
*   return: pointer to created integer_buffer
*/
integer_buffer* integer_buffer_create(size_t length);

/*
*   A method to destroy integer_buffer
*/
void integer_buffer_destroy(integer_buffer* buffer);

/*
*   A method to insert value into integer_buffer
*   Calling this method blocks a thread as long as buffer is full
*/
void integer_buffer_write(integer_buffer* buffer, int value);

/*
*  Return oldest value from integer_buffer and remove it 
*  Calling this method blocka a thread as long as buffer is empty
*/
int integer_buffer_read(integer_buffer* buffer);

#endif