#include <stdio.h>
#include <signal.h>
#include "test/tests.h"
#include "reader.h"
#include "watchdog.h"
#include "watchdog-box.h"
#include "string-buffer.h"
#include <string.h>
#include <pthread.h>
#include "logger.h"
#include <stdlib.h>
#include <stdatomic.h>

reader* reader_object = 0;
watchdog* watchdog_object = 0;

typedef struct dummy_analyzer {
    pthread_t id;
    string_buffer* input;
    string_buffer* logger_buffer;
    watchdog_box* box;
} dummy_analyzer;

void* thread_dummy_analyzer(void* args) {
    dummy_analyzer* dummy_analyzer_object = (dummy_analyzer*) args;

    string_buffer* input = dummy_analyzer_object->input;
    string_buffer* logger_buffer = dummy_analyzer_object->logger_buffer;
    watchdog_box* box = dummy_analyzer_object->box;

    register const int data_length = 255;

    char data[data_length];

    while (1) {
        watchdog_box_click(box);
        string_buffer_read(input, data, data_length);
        
        if (!strcmp(data, "exit")) {
            break;
        }

        string_buffer_write(logger_buffer, "analyzed\n");

        printf("%s", data);
    }

    string_buffer_write(logger_buffer, "exit");
    printf("dummy analyzer exit\n");
    return 0;
}

void signal_handler() {
    if (watchdog_object && reader_object) {
        watchdog_send_exit_signal(watchdog_object);
    }
}

int main() {
    signal(SIGTERM, signal_handler); 
    signal(SIGINT, signal_handler); 
    watchdog_box* reader_box = watchdog_box_create();
    watchdog_box* logger_box = watchdog_box_create();
    watchdog_box* dummy_analyzer_box = watchdog_box_create();

    register const int boxes_length = 3;
    watchdog_box* boxes[] = {reader_box, logger_box, dummy_analyzer_box};

    register const int buffer_capacity = 10;
    string_buffer* buffer = string_buffer_create(buffer_capacity);

    register const int logger_buffer_capacity = 10;
    string_buffer* logger_buffer = string_buffer_create(logger_buffer_capacity);

    dummy_analyzer* dummy_analyzer_object = malloc(sizeof(*dummy_analyzer_object));
    dummy_analyzer_object->box = dummy_analyzer_box;
    dummy_analyzer_object->input = buffer;
    dummy_analyzer_object->logger_buffer = logger_buffer;

    pthread_create(&dummy_analyzer_object->id, NULL, &thread_dummy_analyzer, dummy_analyzer_object);
    
    const char file_name[] = "logger";
    
    logger* logger_object = logger_create(logger_buffer, logger_box, file_name);

    atomic_int program_exit = 0;

    reader_object = reader_create(buffer, reader_box, &program_exit);
  
    watchdog_object = watchdog_create(boxes_length, boxes, &program_exit);

    reader_join(reader_object);
    watchdog_join(watchdog_object);
    pthread_join(dummy_analyzer_object->id, NULL);
    logger_join(logger_object);

    watchdog_box_destroy(reader_box);
    watchdog_box_destroy(logger_box);
    watchdog_box_destroy(dummy_analyzer_box);

    string_buffer_destroy(buffer);
    string_buffer_destroy(logger_buffer);

    reader_destroy(reader_object);
    watchdog_destroy(watchdog_object);
    logger_destroy(logger_object);

    free(dummy_analyzer_object);
    return 0;
}