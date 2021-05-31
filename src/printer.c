#include "printer.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

struct printer {
    integer_buffer* input;
    string_buffer* logger_buffer;
    watchdog_box* box; 
    pthread_t id;
};

void* thread_printer(void* args) {
    printer* printer_object = (printer*) args;
    integer_buffer* input = printer_object->input;
    string_buffer* logger_buffer = printer_object->logger_buffer;
    watchdog_box* box = printer_object->box;

    int cpus = integer_buffer_read(input);
    int cpu_number = 0;

    while (1) {
        watchdog_box_click(box);

        int cpu_usage = integer_buffer_read(input);

        if (cpu_usage == -1) {
            printf("printer exit\n");
            string_buffer_write(logger_buffer, "exit");
            return 0;
        }

        string_buffer_write(logger_buffer, "new value is printed\n");
        printf("cpu%d usage is %d\n", cpu_number, cpu_usage);

        cpu_number = (cpu_number + 1) % cpus;
    }       
}

printer* printer_create(integer_buffer* input, string_buffer* logger_buffer, watchdog_box* box) {
    printer* printer_object = malloc(sizeof(*printer_object));
    printer_object->input = input;
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
