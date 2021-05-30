#include "reader-test.h"
#include <stdatomic.h>
#include <pthread.h>
#include "../reader.h"
#include "../string-buffer.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>

void* create_receiver(void* args) {

    
    string_buffer* buffer = (string_buffer*) args;

    register const size_t data_length = 255;
    char data[data_length];

    while (1) {
        string_buffer_read(buffer, data, data_length);
        if (!strcmp(data, "exit")) {
            return 0;
        }

        printf("%s\n", data);
    } 
}

void reader_test() {
    string_buffer* buffer = string_buffer_create(5);
    pthread_t receiver_id;

    pthread_create(&receiver_id, NULL, &create_receiver, buffer);
    printf("15 second to finish reader\n");
    reader* reader_object = reader_create(buffer);
    sleep(2);
    reader_send_exit_signal(reader_object);
    reader_join(reader_object);
    pthread_join(receiver_id, NULL);

    string_buffer_destroy(buffer);
    reader_destroy(reader_object);

    
}