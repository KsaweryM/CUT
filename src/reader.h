#ifndef __READER_H__
#define __READER_H__

#include "string-buffer.h"
#include "watchdog-box.h"
#include <stdatomic.h>

typedef struct reader reader;

reader* reader_create(string_buffer* output, string_buffer* logger_buffer, watchdog_box* box);

void reader_send_exit_signal(reader* reader_object);

void reader_join(reader* reader_object);

void reader_destroy(reader* reader);
#endif //__READER_H__