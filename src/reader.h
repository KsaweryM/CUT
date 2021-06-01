#ifndef __READER_H__
#define __READER_H__

#include "string-buffer.h"
#include "watchdog-box.h"
#include <stdatomic.h>

/*
* Reader is a thread that reads from /proc/stat file information about processors and sends it to analyzer_buffer.
* At the beginning reader counts how many processors there are in the computer. Let's assume that there are N processors. 
<<<<<<< HEAD
* Reader thread in the loop opens proc/stat file, avoids the first line, reads next N lines and send them to output buffer, close file. 
* 
* Reader thread also informs watchdog thread about itself activity and sends logs info to logger thread. 
=======
* Reader in the loop opens proc/stat file, avoids the first line, reads next N lines and sends them to output buffer, close file. 
>>>>>>> 875bc0abbd33f771f3455835fc1c3719ae827ca8
*/
typedef struct reader reader;

reader* reader_create(string_buffer* analyzer_buffer, string_buffer* logger_buffer, watchdog_box* box);

/*
* Execute this method to terminate reader thread. Reader will ask analyzer thread to terminate itself by analyzer_buffer. Logger thread will be asked to terminate by another thread.
*/
void reader_send_exit_signal(reader* reader_object);

void reader_join(reader* reader_object);

void reader_destroy(reader* reader);

#endif 
