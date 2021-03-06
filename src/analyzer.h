#ifndef ANALYZER_H
#define ANALYZER_H

#include "string-buffer.h"
#include "integer-buffer.h"
#include "watchdog-box.h"

/*
* Body of the analyzer thread. Analyzer is a thread that reads data from reader thread and processes it. 
* Analyzer thread calculates the CPU usages basen on these data and sends them to printer thread.
*/
typedef struct analyzer analyzer;

analyzer* analyzer_create(string_buffer* restrict reader_buffer, string_buffer* restrict logger_buffer, integer_buffer* printer_buffer,  watchdog_box* box, integer_buffer* cpus_count);

void analyzer_destroy(analyzer* analyzer_object);

void analyzer_join(analyzer* analyzer_object);

#endif 
