#ifndef __READER_H__
#define __READER_H__

#include "string-buffer.h"
#include "watchdog-box.h"
#include <stdatomic.h>

/*
* Reader is a thread that read from /proc/stat file information about processors and send it to output buffer.
* At the beginning reader counts how many processors there are in the computer. Let's assume that there are N processors. 
* Reader in the loop opens proc/stat file, avoids the first line, reads next N lines and send them to output buffer, close file. 
*/
typedef struct reader reader;

reader* reader_create(string_buffer* output, string_buffer* logger_buffer, watchdog_box* box);

/*
* Execute this method to terminate reader thread. Reader will ask next thread to terminate itself by output buffer. Logger_buffer will be asked to terminate by another thread.
*/
void reader_send_exit_signal(reader* reader_object);

void reader_join(reader* reader_object);

void reader_destroy(reader* reader);

#endif 