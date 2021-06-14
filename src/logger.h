#ifndef LOGGER_H
#define LOGGER_H

#include "string-buffer.h"
#include "watchdog-box.h"

/*
* Logger is a thread that reads strings from input_buffer and saves them to file.
*/
typedef struct logger logger;

logger* logger_create(string_buffer* input, watchdog_box* box, const char file_name[]);

void logger_join(logger* logger_object);

void logger_destroy(logger* loger_object);

#endif
