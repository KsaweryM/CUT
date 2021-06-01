#include <stdio.h>
#include <stdatomic.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include "reader.h"

/*
* struct reader contains the most important data of reader thread.
*/
struct reader {
    // Set exit value to 1 to exit reader threat. To set this value use "reader_send_exit_signal" method.
    atomic_int exit;
    
    // Buffor for communication with the analyzer thread.
    string_buffer* analyzer_buffer;
    // Buffor for communication with the logger thread.
    string_buffer* logger_buffer;

    // Reader thread uses watchdog_box object to informs watchdog thread about itself activity. 
    watchdog_box* box;
    
    // id of reader thread
    pthread_t id;
};

/*
* Return how many CPUs there are.
*/
size_t cpu_count();

/*
* Body of the reader thread. Reader thread reads data from /proc/stat file and send them to analyzer thread.
* Reader thread also informs watchdog thread about itself activity and sends logs info to logger thread. 
*/
void *thread_reader(void * args) {
    reader* reader_object = (reader*) args;

    string_buffer* analyzer_buffer = reader_object->analyzer_buffer;
    string_buffer* logger_buffer = reader_object->logger_buffer;

    register const size_t data_max_length = 256;
    // data array contains data from /proc/stat file
    char data[data_max_length];

    // number of cpus
    size_t cpus = cpu_count();
    
    // Reader must inform analyzer of the number of cpu.
    // This is done by casting cpus size_t into string and sending it by string_buffer.
    sprintf(data, "%zu", cpus);
    string_buffer_write(analyzer_buffer, data);

    // Reader thread is working until another thread ask it to stop. 
    // This can be done by executing "reader_send_exit_signal" method on right reader object.
    while (!reader_object->exit) {
        // Reader thread sends information to logger thread what it is doing
        string_buffer_write(logger_buffer, "Reader reads the file");
      
        // Reader thread opens /proc/stat file and avoids first line. 
        FILE *file = fopen("/proc/stat", "r");
        assert(file);

        fgets(data, data_max_length, file);

        // Reader thread sends information about processors state to analyzer thread.
        for (size_t i = 0; i < cpus; i++) {
            fgets(data, data_max_length, file);
            string_buffer_write(analyzer_buffer, data);
        }

        fclose(file);
        
        // Reader thread informs watchdog thread about its activity.
        watchdog_box_click(reader_object->box);

        sleep(1);
    }

    // Reader thread is goint to finish its work. It must to informs analyzer thread to does the same.
    string_buffer_write(reader_object->analyzer_buffer, "exit");
    
    printf("reader exit\n");
    
    return 0;
}

reader* reader_create(string_buffer* analyzer_buffer, string_buffer* logger_buffer, watchdog_box* box) {
    reader* reader_object = malloc(sizeof(*reader_object));
    assert(reader_object);

    reader_object->exit = 0;
    reader_object->id = 0;
    reader_object->analyzer_buffer = analyzer_buffer;
    reader_object->logger_buffer = logger_buffer;
    reader_object->box = box;

    pthread_create(&reader_object->id, NULL, &thread_reader, reader_object);

    return reader_object;
}

void reader_send_exit_signal(reader* reader_object) {
    reader_object->exit = 1;
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
size_t cpu_count() {
    FILE* file = fopen("/proc/stat", "r");
    assert(file);

    register const size_t data_max_length = 256;
    char data[data_max_length];
 
    size_t cpus = 0;

    while (fgets(data, data_max_length, file)) {
        if (!strncmp(data, "cpu", 3)) {
            cpus++;
        }
        else {
            break;
        }
    }

    assert(cpus > 0);
    cpus--;

    fclose(file);

    return cpus;
}
