#include <pthread.h>
#include <stdlib.h>
#include <semaphore.h>
#include "integer-buffer.h"

struct integer_buffer {
    pthread_mutex_t mutex;

    sem_t empty;
    sem_t full;
    size_t head, tail;
    size_t length;

    int buffer[];
};

integer_buffer* integer_buffer_create(size_t length) {
    integer_buffer* buffer = malloc(sizeof(*buffer) + sizeof(*buffer->buffer) * length);

    pthread_mutex_init(&buffer->mutex, NULL);

    sem_init(&buffer->empty, 0, (unsigned int) length);
    sem_init(&buffer->full, 0, 0);

    buffer->length = length;
    buffer->head = 0;
    buffer->tail = 0;

    return buffer;
}

void integer_buffer_destroy(integer_buffer* buffer) {
    pthread_mutex_destroy(&buffer->mutex);
    
    sem_destroy(&buffer->empty);
    sem_destroy(&buffer->full);

    free(buffer);
}

void integer_buffer_write(integer_buffer* buffer, int value) {
    sem_wait(&buffer->empty);
    pthread_mutex_lock(&buffer->mutex);

    buffer->buffer[buffer->tail] = value;
    buffer->tail = (buffer->tail + 1) % buffer->length;

    pthread_mutex_unlock(&buffer->mutex);
    sem_post(&buffer->full);
}

int integer_buffer_read(integer_buffer* buffer) {
    sem_wait(&buffer->full);
    pthread_mutex_lock(&buffer->mutex);

    int result = buffer->buffer[buffer->head];
    buffer->head = (buffer->head + 1) % buffer->length;

    pthread_mutex_unlock(&buffer->mutex);
    sem_post(&buffer->empty);

    return result;
}
