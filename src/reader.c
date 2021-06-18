#include <stdio.h>
#include <stdatomic.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include "reader.h"
#include "macros.h"

/*
* Return how many CPUs there are.
*/
static size_t cpu_count(void);


/*
* struct reader contains the most important data of reader thread.
*/
struct reader {
    char padding[4];
    // Set exit value to 1 to exit reader threat. To set this value use "reader_send_exit_signal" method.
    volatile atomic_int exit;
    // Buffer for communication with the analyzer thread.
    string_buffer* analyzer_buffer;
    // Buffer for communication with the logger thread.
    string_buffer* logger_buffer;
    // Reader thread uses this buffer to tell the analyzer threaed how many CPU are. 
    integer_buffer* cpus_count;

    // Reader thread uses watchdog_box object to informs watchdog thread about itself activity. 
    watchdog_box* box;
    
    // Id of reader thread.
    pthread_t id;
};

/*
* Body of the reader thread. Reader thread reads data from /proc/stat file and send them to analyzer thread.
* Reader thread also informs watchdog thread about itself activity and sends logs info to logger thread. 
*/
static void *thread_reader(void * args) {
    reader* reader_object = (reader*) args;

    string_buffer* analyzer_buffer = reader_object->analyzer_buffer;
    string_buffer* logger_buffer = reader_object->logger_buffer;
    watchdog_box* box = reader_object->box;
    integer_buffer* cpus_counts = reader_object->cpus_count;

    // data array contains data from /proc/stat file
    char data[DATA_LENGTH];

    // number of cpus
    register const size_t cpus = cpu_count();
    
    // Reader must inform analyzer of the number of cpu.
    integer_buffer_write(cpus_counts, (int) cpus);

    // Reader thread is working until another thread ask it to stop. 
    // This can be done by executing "reader_send_exit_signal" method on right reader object.
    while (!atomic_load(&reader_object->exit)) {
        // Reader thread sends information to logger thread what it is doing
        string_buffer_write(logger_buffer, "Reader reads the file");
      
        // Reader thread opens /proc/stat file and avoids first line. 
        FILE *file = fopen("/proc/stat", "r");

        if (!file) {
            exit(EXIT_FAILURE);
        }

        fgets(data, DATA_LENGTH, file);

        // Reader thread sends information about states of processors to analyzer thread.
        for (size_t i = 0; i < cpus; i++) {
            fgets(data, DATA_LENGTH, file);
            string_buffer_write(analyzer_buffer, data);
        }

        fclose(file);
        
        // Reader thread informs watchdog thread about its activity.
        watchdog_box_click(box);
        
        sleep(1);
    }

    // Reader thread is goint to finish its work. It must to informs analyzer thread to does the same.
    string_buffer_write(reader_object->analyzer_buffer, STRING_BUFFER_EXIT);
    
    return 0;
}

reader* reader_create(string_buffer* restrict analyzer_buffer, string_buffer* restrict logger_buffer, watchdog_box* box, integer_buffer* cpus_count) {
    reader* reader_object = malloc(sizeof(*reader_object));

    if (!reader_object) {
        exit(EXIT_FAILURE);
    }

    atomic_store(&reader_object->exit, 0);
    reader_object->id = 0;
    reader_object->analyzer_buffer = analyzer_buffer;
    reader_object->logger_buffer = logger_buffer;
    reader_object->box = box;
    reader_object->cpus_count = cpus_count;

    pthread_create(&reader_object->id, NULL, &thread_reader, reader_object);

    return reader_object;
}

void reader_send_exit_signal(reader* reader_object) {
    atomic_store(&reader_object->exit, 1);
}

void reader_join(reader* reader_object) {
    pthread_join(reader_object->id, NULL);
}

void reader_destroy(reader* reader_object) {
    free(reader_object);
}

/*
* Return how many CPUs there are.
*
* It counts how many lines from /proc/stat file begin with a "cpu" prefix and returns this value minus one, because 
* "the very first <<cpu>> line aggregates the numbers in all of the other "cpuN" lines."
*/
static size_t cpu_count() {
    FILE* file = fopen("/proc/stat", "r");

    if (!file) {
        exit(EXIT_FAILURE);
    }

    char data[DATA_LENGTH];
 
    size_t cpus = 0;

    while (fgets(data, DATA_LENGTH, file)) {
        if (!strncmp(data, "cpu", 3)) {
            cpus++;
        }
        else {
            break;
        }
    }

    if (!cpus) {
        exit(EXIT_FAILURE);
    }

    cpus--;

    fclose(file);

    return cpus;
}
