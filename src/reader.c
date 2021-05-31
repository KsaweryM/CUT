#include <stdio.h>
#include <stdatomic.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "reader.h"

struct reader {
    atomic_int* exit;
    
    string_buffer* str_buffer;
    string_buffer* logger_buffer;

    watchdog_box* box;
    
    pthread_t id;
};

int cpu_count();

void *thread_reader(void * args) {
    reader* reader_object = (reader*) args;
    string_buffer* buffer = reader_object->str_buffer;

    register const int data_max_length = 256;
    char data[data_max_length];

    int cpus = cpu_count();
    sprintf(data, "%d", cpus);

    string_buffer_write(buffer, data);

    while (!*reader_object->exit) {
        FILE *file = fopen("/proc/stat", "r");

        fgets(data, data_max_length, file);

        for (int i = 0; i < cpus; i++) {
            fgets(data, data_max_length, file);
            string_buffer_write(buffer, data);
        }

        fclose(file);
        
        watchdog_box_click(reader_object->box);
        sleep(1);
    }

    string_buffer_write(reader_object->str_buffer, "exit");
    
    printf("reader exit\n");
    return 0;
}

reader* reader_create(string_buffer* output, watchdog_box* box, atomic_int* program_exit) {
    reader* reader_object = malloc(sizeof(*reader_object));

    reader_object->exit = program_exit;
    reader_object->id = 0;
    reader_object->str_buffer = output;
    reader_object->box = box;

    pthread_create(&reader_object->id, NULL, &thread_reader, reader_object);

    return reader_object;
}

void reader_destroy(reader* reader) {
    free(reader);
}

void reader_send_exit_signal(reader* reader_object) {
    *reader_object->exit = 1;
}

void reader_join(reader* reader_object) {
    pthread_join(reader_object->id, NULL);
}

int cpu_count() {
    FILE* file = fopen("/proc/stat", "r");

    register const int data_max_length = 256;
    char data[data_max_length];
    int cpus = 0;

    while (fgets(data, data_max_length, file)) {
        if (!strncmp(data, "cpu", 3)) {
            cpus++;
        }
        else {
            break;
        }
    }

    if (cpus > 0)
        cpus--;

    fclose(file);

    return cpus;
}