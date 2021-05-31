#ifndef __PRINTER_H__
#define __PRINTER_H__

#include "integer-buffer.h"
#include "string-buffer.h"
#include "watchdog-box.h"

typedef struct printer printer;

printer* printer_create(integer_buffer* input, string_buffer* logger_buffer, watchdog_box* box);

void printer_destroy(printer* printer_object);

void printer_join(printer* printer_object);

#endif