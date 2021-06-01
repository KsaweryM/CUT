#ifndef __ANALYZER_H__
#define __ANALYZER_H__

#include "string-buffer.h"
#include "integer-buffer.h"
#include "watchdog-box.h"

/*
* Analyzer is a thread that reads data from reader_buffer and processes it. 
* Analyzer thread calculates the CPU usages basen on these data and sends them to printer thread by printer_buffer.
*
*/
typedef struct analyzer analyzer;

analyzer* analyzer_create(string_buffer* reader_buffer, integer_buffer* printer_buffer, string_buffer* logger_buffer, watchdog_box* box);

void analyzer_destroy(analyzer* analyzer_object);

void analyzer_join(analyzer* analyzer_object);


#endif 
