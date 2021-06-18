#ifndef READER_H
#define READER_H

#include "string-buffer.h"
#include "watchdog-box.h"
#include <stdatomic.h>
#include "integer-buffer.h"

/*
* At the beginning reader counts how many processors there are in the computer. Let's assume that there are N processors.
* Reader is a thread that reads from /proc/stat file information about N processors and sends it to analyzer_buffer.
* Reader thread also informs watchdog thread about itself activity and sends logs info to logger thread. 
*/
typedef struct reader reader;

reader* reader_create(string_buffer* restrict analyzer_buffer, string_buffer* restrict logger_buffer, watchdog_box* box, integer_buffer* cpus_count);

/*
* Execute this method to terminate reader thread. Reader will ask analyzer thread to terminate itself by analyzer_buffer. Logger thread will be asked to terminate by another thread.
*/
void reader_send_exit_signal(reader* reader_object);

void reader_join(reader* reader_object);

void reader_destroy(reader* reader);

#endif 
