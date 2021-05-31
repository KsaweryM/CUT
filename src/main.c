#include <stdio.h>
#include <signal.h>
#include "test/tests.h"
#include "reader.h"
#include "watchdog.h"
#include "watchdog-box.h"
#include "string-buffer.h"
#include <string.h>
#include <pthread.h>

reader* reader_object = 0;
watchdog* watchdog_object = 0;

void* dummy_analyzer(void* args) {
    string_buffer* buffer = (string_buffer*) args;
    register const int data_length = 255;

    char data[data_length];

    while (1) {
        string_buffer_read(buffer, data, data_length);
        
        if (!strcmp(data, "exit")) {
            break;
        }

        printf("%s", data);
    }

    return 0;
}

void signal_handler() {
    if (watchdog_object && reader_object) {
        watchdog_send_exit_signal(watchdog_object);
        reader_send_exit_signal(reader_object);
    }
}

int main() {
    signal(SIGTERM, signal_handler); 

    watchdog_box* box = watchdog_box_create();
    register const int boxes_length = 1;
    watchdog_box* boxes[] = {box};

    register const int buffer_capacity = 10;
    string_buffer* buffer = string_buffer_create(buffer_capacity);

    pthread_t dummy_analyzer_id;
    pthread_create(&dummy_analyzer_id, NULL, &dummy_analyzer, buffer);
    
    reader_object = reader_create(buffer, box);

    watchdog_object = watchdog_create(boxes_length, boxes);

    reader_join(reader_object);
    watchdog_join(watchdog_object);
    pthread_join(dummy_analyzer_id, NULL);

    watchdog_box_destroy(box);
    string_buffer_destroy(buffer);
    reader_destroy(reader_object);
    watchdog_destroy(watchdog_object);
    
    return 0;
}