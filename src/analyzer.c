#include "analyzer.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "macros.h"

/*
* Struct cpu_data is useed to store infromation about usage of CPU. 
*/
typedef struct cpu_data {
    unsigned long long int user_time, nice_time, system_time, idle_time, io_wait, irq, soft_irq, steal, guest, guest_nice;
} cpu_data;

/*
* struct analyzer contains the most important data of analyzer thread.
*/
struct analyzer {
    // Buffer for communication with the reader thread.
    string_buffer* input;
    // Buffer for communication with the printer thread.
    integer_buffer* output;
    // Buffer for communication with the logger thread.
    string_buffer* logger_buffer;
    // Analyzer thread uses watchdog_box object to informs watchdog thread about itself activity. 
    watchdog_box* box;
    // Analyzer thread uses "cpus_count" integer_buffer to get number of CPUs from reader.
    integer_buffer* cpus_count;
    // Id of analyzer thread.    
    pthread_t id;
};

/*
* Body of the analyzer thread. Analyzer is a thread that reads data from reader thread and processes it. 
* Analyzer thread calculates the CPU usages basen on these data and sends them to printer thread.
*/
static void* analyzer_thread(void* args) {
    analyzer* analyzer_object = (analyzer*) args;

    string_buffer* input = analyzer_object->input;
    integer_buffer* output = analyzer_object->output;
    string_buffer* logger_buffer = analyzer_object->logger_buffer;
    watchdog_box* box = analyzer_object->box;
    integer_buffer* cpus_count = analyzer_object->cpus_count;

    // Get number of CPUs from reader thread and send it to printer thread.
    register const int cpus_integer = integer_buffer_read(cpus_count);
    register const size_t cpus = (size_t) cpus_integer;
    integer_buffer_write(output, cpus_integer);

    // cpus_current_data is used to store current state of CPUs.
    cpu_data* cpus_current_data = malloc(sizeof(*cpus_current_data) * cpus);
    // cpus_previous_data is used to store previous state of CPUs.
    cpu_data* cpus_previous_data = malloc(sizeof(*cpus_previous_data) * cpus);

    // Analyzer thread sends information to logger thread what it is doing.
    string_buffer_write(logger_buffer, ANALYZER_LOG_MESSAGE_1);

    // Analyzer thread informs watchdog thread about its activity.
    watchdog_box_click(box); 

    // The data array is used to store data from reader thread. 
    char data[DATA_LENGTH];

    // To calculate the CPUs usage, we need to count differences on the current and previous state of CPUs.
    // Analyzer thread received the current state of CPUs, but there is no previous state of CPUs.
    // Analyzer thread stores the current state of CPUs as the previous one and waits for next state of CPUs, to calculate usage of CPUs. 
    for (size_t i = 0; i < cpus; i++) {
        // Analyzer thread receives data from reader thread. 
        string_buffer_read(input, data, DATA_LENGTH);

        // Analyzer thread receives a request to finish its work. 
        // Analyzer thread must dealocate memory and ask printer thread to finish its work. 
        if (!strcmp(data, STRING_BUFFER_EXIT)) {
            integer_buffer_write(output, INTEGER_BUFFER_EXIT);

            free(cpus_previous_data);
            free(cpus_current_data);

            return 0;
        }

        sscanf(data, "cpu%*d %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu", &cpus_previous_data[i].user_time, &cpus_previous_data[i].nice_time, &cpus_previous_data[i].system_time, &cpus_previous_data[i].idle_time, &cpus_previous_data[i].io_wait, &cpus_previous_data[i].irq, &cpus_previous_data[i].soft_irq, &cpus_previous_data[i].steal, &cpus_previous_data[i].guest, &cpus_previous_data[i].guest_nice);
    }
   
    while (1) {
         // Analyzer thread informs watchdog thread about its activity.
        watchdog_box_click(box);

        for (size_t i = 0; i < cpus; i++) {
            // Analyzer thread receives data from reader thread. 
            string_buffer_read(input, data, DATA_LENGTH);

            // Analyzer thread receives a request to finish its work. 
            // Analyzer thread must dealocate memory and ask printer thread to finish its work. 
            if (!strcmp(data, STRING_BUFFER_EXIT)) {
                 integer_buffer_write(output, INTEGER_BUFFER_EXIT);

                 free(cpus_previous_data);
                 free(cpus_current_data);

                 return 0;
            }

            sscanf(data, "cpu%*d %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu", &cpus_current_data[i].user_time, &cpus_current_data[i].nice_time, &cpus_current_data[i].system_time, &cpus_current_data[i].idle_time, &cpus_current_data[i].io_wait, &cpus_current_data[i].irq, &cpus_current_data[i].soft_irq, &cpus_current_data[i].steal, &cpus_current_data[i].guest, &cpus_current_data[i].guest_nice);
        }

        // Analyzer thread counts the usage of particular CPU and sends its to printer thread. 
        for (size_t i = 0; i < cpus; i++) {
            unsigned long long int Prev_idle = cpus_previous_data[i].idle_time + cpus_previous_data[i].io_wait;
            unsigned long long int Idle = cpus_current_data[i].idle_time + cpus_current_data[i].io_wait;

            unsigned long long int Prev_non_idle = cpus_previous_data[i].user_time + cpus_previous_data[i].nice_time + cpus_previous_data[i].system_time + cpus_previous_data[i].irq + cpus_previous_data[i].soft_irq + cpus_previous_data[i].steal;
            unsigned long long int Non_idle      = cpus_current_data[i].user_time  + cpus_current_data[i].nice_time  + cpus_current_data[i].system_time  + cpus_current_data[i].irq  + cpus_current_data[i].soft_irq  + cpus_current_data[i].steal;

            unsigned long long int Prev_total = Prev_idle + Prev_non_idle;
            unsigned long long int Total = Idle + Non_idle;

            unsigned long long int total_d = Total - Prev_total;
            unsigned long long int idle_d  = Idle - Prev_idle;
       
            int CPU_Percentage = (total_d == 0) ? 0 : (int) (100 * (total_d - idle_d) /  total_d);

            string_buffer_write(analyzer_object->logger_buffer, ANALYZER_LOG_MESSAGE_2);
            integer_buffer_write(output, CPU_Percentage);
        }

        memcpy(cpus_previous_data, cpus_current_data, sizeof(*cpus_previous_data) * cpus);
    }
}

analyzer* analyzer_create(string_buffer* restrict reader_buffer, string_buffer* restrict logger_buffer, integer_buffer* printer_buffer,  watchdog_box* box, integer_buffer* cpus_count) {
    analyzer* analyzer_object = malloc(sizeof(*analyzer_object));

    if (!analyzer_object) {
        exit(EXIT_FAILURE);
    }

    analyzer_object->input = reader_buffer;
    analyzer_object->logger_buffer = logger_buffer;
    analyzer_object->output = printer_buffer;
    analyzer_object->box = box;
    analyzer_object->cpus_count = cpus_count;

    pthread_create(&analyzer_object->id, NULL, &analyzer_thread, analyzer_object);
    return analyzer_object;
}

void analyzer_destroy(analyzer* analyzer_object) {
    free(analyzer_object);
}

void analyzer_join(analyzer* analyzer_object) {
    pthread_join(analyzer_object->id, NULL);
}
