#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <string.h>
#include "reader.h"
#include "watchdog.h"
#include "watchdog-box.h"
#include "string-buffer.h"
#include "logger.h"
#include "analyzer.h"
#include "printer.h"
#include "signal-handler.h"
#include "macros.h"

/*
* When the program receives the SIGTERM signal, it starts to terminates threads.
*/
int main() {
    signal(SIGTERM, signal_handler);

    register const size_t reader_analyzer_buffer_capacity = 10;
    register const size_t analyzer_printer_buffer_capacity = 30;
    register const size_t logger_buffer_capacity = 20;

    register const size_t word_length = 512;
    register const size_t cpus_count_buffer_capacity = 1;

    /*
    * Creating buffers. Buffers are used to communication between threads.
    */
    string_buffer* reader_analyzer_buffer = string_buffer_create(reader_analyzer_buffer_capacity, word_length);
    integer_buffer* analyzer_printer_buffer = integer_buffer_create(analyzer_printer_buffer_capacity);
    string_buffer* logger_buffer = string_buffer_create(logger_buffer_capacity, word_length);

    /*
    * Reader thread uses this buffer to tell the analyzer threaed how many CPU are. 
    */
    integer_buffer* cpus_count = integer_buffer_create(cpus_count_buffer_capacity);

    /*
    * Creating watchdog_boxes. Every thread must inform watchdog thread abouts its activity. 
    * To inform watchdog threat about its activity, every thread must execute watchdog_box_click(watchdog_box) at least every 2 seconds.
    */
    watchdog_box* reader_box = watchdog_box_create();
    watchdog_box* analyzer_box = watchdog_box_create();
    watchdog_box* printer_box = watchdog_box_create();
    watchdog_box* logger_box = watchdog_box_create();
 
    /*
    * Array of pointers to watchdog_boxes is a one of the argument of watchdog thread. 
    * For every 2 seconds watchdogs checks if there was a activity on every watchdog_boxes. 
    * If there was no activity on one of the boxes, watchdog abort the program. 
    */
    watchdog_box* boxes[] = {reader_box, analyzer_box, printer_box, logger_box};
    size_t boxes_length = sizeof(boxes) / sizeof(boxes[0]);

    /*
    * Creating threads. 
    */
    reader* reader_object = reader_create(reader_analyzer_buffer, logger_buffer, reader_box, cpus_count);
    analyzer* analyzer_objet = analyzer_create(reader_analyzer_buffer, logger_buffer, analyzer_printer_buffer, analyzer_box, cpus_count);
    printer* printer_object = printer_create(analyzer_printer_buffer, logger_buffer, printer_box);
    logger* logger_object = logger_create(logger_buffer, logger_box, LOGGER_FILE_NAME);    
    watchdog* watchdog_object = watchdog_create(boxes_length, boxes);

    /*
    * By signals handling user can terminate the program. Each thread must be correctly terminated.
    */
    watchdog_join(watchdog_object);
    
    /*
    * The next thread to terminate is reader. Reader sends information about termination to analyzer thread by "reader_analyzer_buffer" 
    * Analyzer thread does the same for printer thread and printer thread informs logger thread.
    */
    reader_send_exit_signal(reader_object);

    reader_join(reader_object);
    analyzer_join(analyzer_objet);
    printer_join(printer_object);
    logger_join(logger_object);

    /*
    * In the following lines program deallocates memory.
    */
    reader_destroy(reader_object);
    analyzer_destroy(analyzer_objet);
    printer_destroy(printer_object);
    logger_destroy(logger_object);
    watchdog_destroy(watchdog_object);

    string_buffer_destroy(reader_analyzer_buffer);
    string_buffer_destroy(logger_buffer);
    integer_buffer_destroy(analyzer_printer_buffer);
    integer_buffer_destroy(cpus_count);

    watchdog_box_destroy(reader_box);
    watchdog_box_destroy(analyzer_box);
    watchdog_box_destroy(printer_box);
    watchdog_box_destroy(logger_box);
 
    return 0;
}
