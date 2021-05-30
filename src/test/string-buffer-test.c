
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include "../string-buffer.h"
#include "string-buffer-test.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

static void random_string(char *str, const int str_length) {
    for (int i = 0; i < str_length - 1; ++i) {
        str[i] = (char) ((rand() % ('Z' - 'A' + 1)) + 'A');
    }

    str[str_length - 1] = 0;
}

char** string_array(const int length, const int string_length) {
    char** data =  malloc(sizeof(*data) * length);

       for (int i = 0; i < length; ++i) {
        data[i] = malloc(sizeof(*data[i]) * string_length);
    }

    return data;
}

char** random_string_array(const int length, const int string_length) {
    char** data = string_array(length, string_length);

    for (int i = 0; i < length; ++i) {
        random_string(data[i], string_length);
    }

    return data;
}

typedef struct thread_input {
    int data_length;
    int string_length;
    char** data;
    string_buffer* buffer;
} thread_input;

void* string_producer(void* args) {
    thread_input* th = (thread_input*) args;

    string_buffer* buffer = th->buffer;
    char** input = th->data; 
    int data_length = th->data_length;

    for (int i = 0; i < data_length; i++) {
        string_buffer_write(buffer, input[i]);
    }

    return 0;
}

void* string_consumer(void* args) {
    thread_input* th = (thread_input*) args;

    string_buffer* buffer = th->buffer;
    char** output = th->data; 
    int data_length = th->data_length; 
    int string_length = th->string_length;

    for (int i = 0; i < data_length; i++) {
        string_buffer_read(buffer, output[i], string_length);
    }

    return 0;
}

void string_buffer_test() {
    srand(time(NULL));

    pthread_t producerID, consumerID;
    string_buffer* buffer = string_buffer_create(10);

    register const int length = 50;
    register const int string_length = 50;

    char** input = random_string_array(length, string_length);

    char** output = string_array(length, string_length);

    thread_input producer_input = (thread_input) {length, string_length, input, buffer};
    thread_input consumer_input = (thread_input) {length, string_length, output, buffer};

    pthread_create(&producerID, NULL, &string_producer, &producer_input);
    pthread_create(&consumerID, NULL, &string_consumer, &consumer_input);

    pthread_join(producerID, NULL);
    pthread_join(consumerID, NULL);

    for (int i = 0; i < length; ++i) {
        assert(!strcmp(input[i], output[i]));
    }

    for (int i = 0; i < length; i++) {
        free(input[i]);
        free(output[i]);
    }   

    free(input);
    free(output);

    string_buffer_destroy(buffer);
}