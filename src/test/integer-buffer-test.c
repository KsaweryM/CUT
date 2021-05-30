#include <pthread.h>
#include "../integer-buffer.h"
#include "../string-buffer.h"
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

typedef struct thread_input {
    int data_length;
    int* data;
    integer_buffer* buffer;
} thread_input;

void* integer_producer(void* args) {
    thread_input* th = (thread_input*) args;

    integer_buffer* buffer = th->buffer;
    int* input = th->data; 
    int data_length = th->data_length;

    for (int i = 0; i < data_length; i++) {
        integer_buffer_write(buffer, input[i]);
    }

    return 0;
}

void* integer_consumer(void* args) {
    thread_input* th = (thread_input*) args;

    integer_buffer* buffer = th->buffer;
    int* output = th->data; 
    int data_length = th->data_length; 

    for (int i = 0; i < data_length; i++) {
        output[i] = integer_buffer_read(buffer);
    }

    return 0;
}

void integer_buffer_test() {
    srand(time(NULL));

    pthread_t producerID, consumerID;
    integer_buffer* buffer = integer_buffer_create(10);

    register const int data_length = 1000;
    
    int* input = malloc(sizeof(*input) * data_length);
    int* output = malloc(sizeof(*output) * data_length);

    for (int i = 0; i < data_length; i++) {
        input[i] = rand();
    }

    thread_input producer_input = (thread_input) {data_length, input, buffer};
    thread_input consumer_input = (thread_input) {data_length, output, buffer};

    pthread_create(&producerID, NULL, &integer_producer, &producer_input);
    pthread_create(&consumerID, NULL, &integer_consumer, &consumer_input);

    pthread_join(producerID, NULL);
    pthread_join(consumerID, NULL);


    assert(!memcmp(input, output, sizeof(*output) * data_length));

    integer_buffer_destroy(buffer);
    free(input);
    free(output);   
}