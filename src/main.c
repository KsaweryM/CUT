#include <stdio.h>
#include <signal.h>
#include "test/tests.h"
#include "reader.h"
#include "watchdog.h"
#include "watchdog-box.h"
#include "string-buffer.h"
#include <string.h>
#include <pthread.h>
#include "logger.h"
#include <stdlib.h>
#include <stdatomic.h>
#include "analyzer.h"
#include "printer.h"

watchdog* watchdog_object = 0;

void signal_handler() {
    if (watchdog_object) {
        watchdog_send_exit_signal(watchdog_object);
    }
}

int main() {
    signal(SIGTERM, signal_handler); 
    signal(SIGINT, signal_handler); 

    register const int reader_analyzer_buffer_capacity = 10;
    register const int logger_buffer_capacity = 20;
    register const int integer_bufer_capacity = 30;

    string_buffer* reader_analyzer_buffer = string_buffer_create(reader_analyzer_buffer_capacity);
    string_buffer* logger_buffer = string_buffer_create(logger_buffer_capacity);
    integer_buffer* analyzer_printer_buffer = integer_buffer_create(integer_bufer_capacity);

    watchdog_box* reader_box = watchdog_box_create();
    watchdog_box* analyzer_box = watchdog_box_create();
    watchdog_box* printer_box = watchdog_box_create();
    watchdog_box* logger_box = watchdog_box_create();
 
    watchdog_box* boxes[] = {reader_box, analyzer_box, printer_box, logger_box};
    int boxes_length = sizeof(boxes) / sizeof(boxes[0]);
    
    atomic_int program_exit = 0;

    reader* reader_object = reader_create(reader_analyzer_buffer, logger_buffer, reader_box, &program_exit);
    analyzer* analyzer_objet = analyzer_create(reader_analyzer_buffer, analyzer_printer_buffer, logger_buffer, analyzer_box);
    printer* printer_object = printer_create(analyzer_printer_buffer, logger_buffer, printer_box);
    logger* logger_object = logger_create(logger_buffer, logger_box, "logger file");    
    watchdog_object = watchdog_create(boxes_length, boxes, &program_exit);

    reader_join(reader_object);
    analyzer_join(analyzer_objet);
    printer_join(printer_object);
    logger_join(logger_object);
    watchdog_join(watchdog_object);

    reader_destroy(reader_object);
    analyzer_destroy(analyzer_objet);
    printer_destroy(printer_object);
    logger_destroy(logger_object);
    watchdog_destroy(watchdog_object);

    string_buffer_destroy(reader_analyzer_buffer);
    string_buffer_destroy(logger_buffer);
    integer_buffer_destroy(analyzer_printer_buffer);

    watchdog_box_destroy(reader_box);
    watchdog_box_destroy(analyzer_box);
    watchdog_box_destroy(printer_box);
    watchdog_box_destroy(logger_box);
 
    return 0;
}