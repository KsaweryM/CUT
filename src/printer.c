#include "printer.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

struct printer {
    integer_buffer* input;
    pthread_t id;
};

void* thread_printer(void* args) {
 printer* printer_object = (printer*) args;

       
}

printer* printer_create(integer_buffer* input) {

}

void printer_destroy(printer* printer) {

}

void printer_join(printer* printer) {

}
