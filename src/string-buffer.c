#include "string-buffer.h"
#include <pthread.h>
#include <stdlib.h>
#include <semaphore.h>
#include <string.h>

struct string_buffer {
    pthread_mutex_t mutex;

    sem_t empty;
    sem_t full;

    size_t head, tail;
    size_t length;

    char* buffer[];
};

string_buffer* string_buffer_create(size_t length) {
    string_buffer* buffer = malloc(sizeof(*buffer) + length * sizeof(*buffer->buffer));

    memset(buffer->buffer, 0, sizeof(*buffer->buffer) * length);
    pthread_mutex_init(&buffer->mutex, NULL);

    sem_init(&buffer->empty, 0, length);
    sem_init(&buffer->full, 0, 0);

    buffer->length = length;

    buffer->head = 0;
    buffer->tail = 0;

    return buffer;
}

void string_buffer_destroy(string_buffer* buffer) {
    pthread_mutex_destroy(&buffer->mutex);
    
    sem_destroy(&buffer->empty);
    sem_destroy(&buffer->full);

    for (size_t i = 0; i < buffer->length; i++) {
        free(buffer->buffer[i]);
    }

    free(buffer);
}

void string_buffer_write(string_buffer* buffer, const char data[]) {
    sem_wait(&buffer->empty);
    pthread_mutex_lock(&buffer->mutex);

    size_t size = strlen(data) + 1;
    buffer->buffer[buffer->tail] = malloc(size);
    memcpy(buffer->buffer[buffer->tail], data, size);
    
    buffer->tail = (buffer->tail + 1) % buffer->length;

    pthread_mutex_unlock(&buffer->mutex);
    sem_post(&buffer->full);
}

size_t string_buffer_read(string_buffer* buffer, char* data, size_t available_space) {
    sem_wait(&buffer->full);
    pthread_mutex_lock(&buffer->mutex);

    size_t string_size = strlen(buffer->buffer[buffer->head]) + 1;
    size_t size = (available_space < string_size) ? available_space : string_size; 

    memcpy(data, buffer->buffer[buffer->head], size);
    free(buffer->buffer[buffer->head]);
    buffer->buffer[buffer->head] = 0;

    buffer->head = (buffer->head + 1) % buffer->length;

    pthread_mutex_unlock(&buffer->mutex);
    sem_post(&buffer->empty);

    return size;
}


