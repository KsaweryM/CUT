#ifndef __ANALYZER_H__
#define __ANALYZER_H__

#include "string-buffer.h"
#include "integer-buffer.h"
#include "watchdog-box.h"

typedef struct analyzer analyzer;

analyzer* analyzer_create(string_buffer* input, integer_buffer* output, string_buffer* logger_buffer, watchdog_box* box);

void analyzer_destroy(analyzer* analyzer_object);

void analyzer_join(analyzer* analyzer_object);


#endif 
