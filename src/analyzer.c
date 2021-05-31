#include "analyzer.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef struct cpu_data {
    unsigned long long int user_time, nice_time, system_time, idle_time;
    unsigned long long int io_wait, irq, soft_irq, steal, guest, guest_nice;
} cpu_data;

struct analyzer {
    string_buffer* input;
    integer_buffer* output;
    string_buffer* logger_buffer;
    watchdog_box* box;
    pthread_t id;
};

void* analyzer_thread(void* args) {
    analyzer* analyzer_object = (analyzer*) args;

    string_buffer* input = analyzer_object->input;
    integer_buffer* output = analyzer_object->output;
    string_buffer* logger_buffer = analyzer_object->logger_buffer;
    watchdog_box* box = analyzer_object->box;

    register const int data_max_length = 256;

    char data[data_max_length];

    string_buffer_read(input, data, data_max_length);

    int cpus = atoi(data);
    integer_buffer_write(output, cpus);

    cpu_data cpus_current_data[cpus];
    cpu_data cpus_previous_data[cpus];

    int first_loop = 1;

    while (1) {
        watchdog_box_click(box);

        if (first_loop) {
            string_buffer_write(logger_buffer, "first loop by analyzer\n");
            for (int i = 0; i < cpus; i++) {
                string_buffer_read(input, data, data_max_length);

                if (i == 0 && !strcmp(data, "exit")) {
                    printf("Analyzer end");
                    integer_buffer_write(output, -1);

                    return 0;

                }

                sscanf(data, "cpu%*d %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu", &cpus_previous_data[i].user_time, &cpus_previous_data[i].nice_time, &cpus_previous_data[i].system_time, &cpus_previous_data[i].idle_time, &cpus_previous_data[i].io_wait, &cpus_previous_data[i].irq, &cpus_previous_data[i].soft_irq, &cpus_previous_data[i].steal, &cpus_previous_data[i].guest, &cpus_previous_data[i].guest_nice);
            }

            first_loop = 0;

            watchdog_box_click(box);
        }

        for (int i = 0; i < cpus; i++) {
            string_buffer_read(input, data, data_max_length);

            if (i == 0 && !strcmp(data, "exit")) {
                 printf("Analyzer exit\n");
                 integer_buffer_write(output, -1);

                 return 0;
            }

            sscanf(data, "cpu%*d %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu", &cpus_current_data[i].user_time, &cpus_current_data[i].nice_time, &cpus_current_data[i].system_time, &cpus_current_data[i].idle_time, &cpus_current_data[i].io_wait, &cpus_current_data[i].irq, &cpus_current_data[i].soft_irq, &cpus_current_data[i].steal, &cpus_current_data[i].guest, &cpus_current_data[i].guest_nice);
        }

        if (!memcmp(cpus_previous_data, cpus_current_data, sizeof(*cpus_previous_data) * cpus)) {
            continue;
        }

        for (int i = 0; i < cpus; i++) {
            unsigned long long int Prev_idle = cpus_previous_data[i].idle_time + cpus_previous_data[i].io_wait;
            unsigned long long int Idle = cpus_current_data[i].idle_time + cpus_current_data[i].io_wait;

            unsigned long long int Prev_non_idle = cpus_previous_data[i].user_time + cpus_previous_data[i].nice_time + cpus_previous_data[i].system_time + cpus_previous_data[i].irq + cpus_previous_data[i].soft_irq + cpus_previous_data[i].steal;
            unsigned long long int Non_idle      = cpus_current_data[i].user_time  + cpus_current_data[i].nice_time  + cpus_current_data[i].system_time  + cpus_current_data[i].irq  + cpus_current_data[i].soft_irq  + cpus_current_data[i].steal;

            unsigned long long int Prev_total = Prev_idle + Prev_non_idle;
            unsigned long long int Total = Idle + Non_idle;

            unsigned long long int total_d = Total - Prev_total;
            unsigned long long int idle_d  = Idle - Prev_idle;

            double CPU_Percentage = total_d == 0 ? 0 : (double) (total_d - idle_d) / total_d;

            integer_buffer_write(output, (int) (CPU_Percentage * 100));
            string_buffer_write(analyzer_object->logger_buffer, "New CPU usage calculated");
        }

        memcpy(cpus_previous_data, cpus_current_data, sizeof(*cpus_previous_data) * cpus);
    }
}

analyzer* analyzer_create(string_buffer* input, integer_buffer* output, string_buffer* logger_buffer, watchdog_box* box) {
    analyzer* analyzer_object = malloc(sizeof(*analyzer_object));

    analyzer_object->input = input;
    analyzer_object->output = output;
    analyzer_object->logger_buffer = logger_buffer;
    analyzer_object->box = box;

    pthread_create(&analyzer_object->id, NULL, &analyzer_thread, analyzer_object);
    return analyzer_object;
}

void analyzer_destroy(analyzer* analyzer_object) {
    free(analyzer_object);
}

void analyzer_join(analyzer* analyzer_object) {
    pthread_join(analyzer_object->id, NULL);
}