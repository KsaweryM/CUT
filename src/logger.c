#include "logger.h"
#include <pthread.h>
#include <stdio.h>
#include "watchdog-box.h"
#include <stdlib.h>
#include "string.h"

struct logger {
    string_buffer* input;
    watchdog_box* box;

    pthread_t id;
    char file_name[];
};

void* thread_logger(void* args) {
    logger* logger_object = (logger*) args;

    string_buffer* input = logger_object->input;
    watchdog_box* box = logger_object->box;

    register const size_t data_size = 255;
    char data[data_size];

    FILE* file = fopen(logger_object->file_name, "w+");

    while (1) {
        watchdog_box_click(box);

        string_buffer_read(input, data, data_size);

        if (!strcmp(data, "exit")) {
            break;
        }
      
        fprintf(file, "%s\n", data);
    }

    fclose(file);

    printf("logger exit\n");
    return 0;
}

logger* logger_create(string_buffer* input, watchdog_box* box, const char file_name[]) {
    size_t file_name_length = strlen(file_name) + 1;
    logger* logger_object = malloc(sizeof(*logger_object) + file_name_length);

    logger_object->input = input;
    logger_object->box = box;

    memcpy(&logger_object->file_name, file_name, file_name_length);

    pthread_create(&logger_object->id, NULL, &thread_logger, logger_object);

    return logger_object;
}

void logger_join(logger* logger_object) {
    pthread_join(logger_object->id, NULL);
}

void logger_destroy(logger* loger_object) {
    free(loger_object);    
}
