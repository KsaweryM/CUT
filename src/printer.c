#include "printer.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

/*
* struct printer contains the most important data of printer thread.
*/
struct printer {
    // Buffor for communication with the analyzer thread. 
    integer_buffer* analyzer_buffer;
    // Boffor for communication with the logger thread.
    string_buffer* logger_buffer;

    // Printer thread uses watchdog_box object to informs watchdog thread about itself activity.
    watchdog_box* box; 

    // id of printer thread.
    pthread_t id;
};

/*
* Body of printer thread. Printer thread reads data from analyzer_buffer and prints them on screen.
* Like Reader thread, printer thread also informs watchdog thread about itself activity and sends logs info to logger thread.
*/

static void* thread_printer(void* args) {
    printer* printer_object = (printer*) args;

    integer_buffer* analyzer_buffer = printer_object->analyzer_buffer;
    string_buffer* logger_buffer = printer_object->logger_buffer;
    
    watchdog_box* box = printer_object->box;

    // Get number of CPUS from analyzer thread.
    int cpus = integer_buffer_read(analyzer_buffer);
    int cpu_number = 0;

    while (1) {
        // Printer thread informs watchdog thread about its activity.
        watchdog_box_click(box);

        // Get cpu usage from analyzer thread.
        int cpu_usage = integer_buffer_read(analyzer_buffer);

        // if cpu_usage equals -1, it means that the analyzer thread finished its work and printer must to exit.
        // zamien -1 na exit_value_int i podobnie z "exit"
        if (cpu_usage == -1) {
            printf("printer exit\n");
            
            // Printer threads asks logger thread to exit.
            string_buffer_write(logger_buffer, "exit");
            return 0;
        }

        // Printer thread sends information to logger thread what is is doing.
        string_buffer_write(logger_buffer, "New value is printed\n");
        // Printer thread prints cpu usage on screen.
        printf("cpu%d usage is %d\n", cpu_number, cpu_usage);

        cpu_number = (cpu_number + 1) % cpus;
    }       
}

printer* printer_create(integer_buffer* analyzer_buffer, string_buffer* logger_buffer, watchdog_box* box) {
    printer* printer_object = malloc(sizeof(*printer_object));
    printer_object->analyzer_buffer = analyzer_buffer;
    printer_object->logger_buffer = logger_buffer;
    printer_object->box = box;

    pthread_create(&printer_object->id, NULL, &thread_printer, printer_object);
   
    return printer_object;
}


void printer_destroy(printer* printer_object) {
    free(printer_object);
}

void printer_join(printer* printer_object) {
    pthread_join(printer_object->id, NULL);
}
