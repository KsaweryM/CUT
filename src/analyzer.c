#include "analyzer.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "macros.h"


typedef struct cpu_data {
    unsigned long long int user_time, nice_time, system_time, idle_time;
    unsigned long long int io_wait, irq, soft_irq, steal, guest, guest_nice;
} cpu_data;

struct analyzer {
    string_buffer* input;
    
    integer_buffer* output;
    
    string_buffer* logger_buffer;
    
    watchdog_box* box;

    integer_buffer* cpus_count;
    
    pthread_t id;
};

static void* analyzer_thread(void* args) {
    analyzer* analyzer_object = (analyzer*) args;

    string_buffer* input = analyzer_object->input;
    integer_buffer* output = analyzer_object->output;
    string_buffer* logger_buffer = analyzer_object->logger_buffer;
    watchdog_box* box = analyzer_object->box;
    integer_buffer* cpus_count = analyzer_object->cpus_count;

    char data[DATA_LENGTH];

    string_buffer_read(input, data, DATA_LENGTH);

    register const int cpus_integer = integer_buffer_read(cpus_count);
    integer_buffer_write(output, cpus_integer);
    register const size_t cpus = (size_t) cpus_integer;

    cpu_data* cpus_current_data = malloc(sizeof(*cpus_current_data) * cpus);
    cpu_data* cpus_previous_data = malloc(sizeof(*cpus_previous_data) * cpus);

    string_buffer_write(logger_buffer, "first loop by analyzer\n");

    watchdog_box_click(box); 

    for (size_t i = 0; i < cpus; i++) {
        string_buffer_read(input, data, DATA_LENGTH);

        if (!strcmp(data, STRING_BUFFER_EXIT)) {
            integer_buffer_write(output, INTEGER_BUFFER_EXIT);

            free(cpus_previous_data);
            free(cpus_current_data);

            return 0;

        }

        sscanf(data, "cpu%*d %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu", &cpus_previous_data[i].user_time, &cpus_previous_data[i].nice_time, &cpus_previous_data[i].system_time, &cpus_previous_data[i].idle_time, &cpus_previous_data[i].io_wait, &cpus_previous_data[i].irq, &cpus_previous_data[i].soft_irq, &cpus_previous_data[i].steal, &cpus_previous_data[i].guest, &cpus_previous_data[i].guest_nice);
    }
   
    while (1) {
        watchdog_box_click(box);

        for (size_t i = 0; i < cpus; i++) {
            string_buffer_read(input, data, DATA_LENGTH);

            if (i == 0 && !strcmp(data, STRING_BUFFER_EXIT)) {
                 printf("Analyzer exit\n");
                 integer_buffer_write(output, INTEGER_BUFFER_EXIT);

                 free(cpus_previous_data);
                 free(cpus_current_data);

                 return 0;
            }

            sscanf(data, "cpu%*d %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu", &cpus_current_data[i].user_time, &cpus_current_data[i].nice_time, &cpus_current_data[i].system_time, &cpus_current_data[i].idle_time, &cpus_current_data[i].io_wait, &cpus_current_data[i].irq, &cpus_current_data[i].soft_irq, &cpus_current_data[i].steal, &cpus_current_data[i].guest, &cpus_current_data[i].guest_nice);
        }

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

            integer_buffer_write(output, CPU_Percentage);
            string_buffer_write(analyzer_object->logger_buffer, "New CPU usage calculated");
        }

        memcpy(cpus_previous_data, cpus_current_data, sizeof(*cpus_previous_data) * (size_t) cpus);
    }
}

analyzer* analyzer_create(string_buffer* restrict reader_buffer, string_buffer* restrict logger_buffer, integer_buffer* printer_buffer,  watchdog_box* box, integer_buffer* cpus_count) {
    analyzer* analyzer_object = malloc(sizeof(*analyzer_object));

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
