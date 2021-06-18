#include "logger.h"
#include <pthread.h>
#include <stdio.h>
#include "watchdog-box.h"
#include <stdlib.h>
#include "string.h"
#include "macros.h"

/*
* struct logger contains the most important data of logger thread.
*/
struct logger {
    // Logger thread reads string from this string_buffer and saves them to file.
    string_buffer* input;

    // Logger thread uses watchdog_box object to informs watchdog thread about itself activity. 
    watchdog_box* box;

    // Id of logger thread.
    pthread_t id;

    // Name of the file to which logger thread saves data.
    char file_name[];
};

/*
* Body of the logger thread. Logger thread reads strings from input_buffer and saves them to file.
* Like Reader thread, logger thread also informs watchdog thread about itself activity.
*/

static void* thread_logger(void* args) {
    logger* logger_object = (logger*) args;

    string_buffer* input = logger_object->input;
    watchdog_box* box = logger_object->box;

    char data[DATA_LENGTH];

    // file to which logger thread saves data.
    FILE* file = fopen(logger_object->file_name, "w+");

    while (1) {
        // Logger thread informs watchdog thread about its activity.
        watchdog_box_click(box);

        // Logger thread saves strings from input buffer
        string_buffer_read(input, data, DATA_LENGTH);

        // data equals "exit", it means that previous threads (reader, analyzer, printer) have finished their work and logger thread must exit.
        if (!strcmp(data, STRING_BUFFER_EXIT)) {
            break;
        }
      
        // Logger thread saves log data into file. 
        fprintf(file, "%s\n", data);
    }

    fclose(file);

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
