#ifndef __PRINTER_H__
#define __PRINTER_H__

#include "integer-buffer.h"
#include "string-buffer.h"
#include "watchdog-box.h"

/*
* Printer is a thread that reads CPU usages from analyzer thread and prints them on screen.
* Like Reader thread, printer thread also informs watchdog thread about itself activity and sends logs info to logger thread.
*/
typedef struct printer printer;

printer* printer_create(integer_buffer* analyzer_buffer, string_buffer* logger_buffer, watchdog_box* box);

void printer_destroy(printer* printer_object);

void printer_join(printer* printer_object);

#endif