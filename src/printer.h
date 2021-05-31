#ifndef __PRINTER_H__
#define __PRINTER_H__

#include "integer-buffer.h"

typedef struct printer printer;

printer* printer_create(integer_buffer* input);

void printer_destroy(printer* printer);

void printer_join(printer* printer);

#endif