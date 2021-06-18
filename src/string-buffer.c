#include "string-buffer.h"
#include <pthread.h>
#include <stdlib.h>
#include <semaphore.h>
#include <string.h>
#include "string-buffer.h"

struct string_buffer {
    pthread_mutex_t mutex;

    sem_t empty;
    sem_t full;

    size_t head, tail, words_count, word_length;

    char buffer[];
};

string_buffer* string_buffer_create(const size_t words_count, const size_t word_length) {
    string_buffer* buffer = malloc(sizeof(*buffer) + words_count * word_length);

    if (!buffer) {
        exit(EXIT_FAILURE);
    }

    pthread_mutex_init(&buffer->mutex, NULL);

    sem_init(&buffer->empty, 0, (unsigned int) words_count);
    sem_init(&buffer->full, 0, 0);

    buffer->head = 0;
    buffer->tail = 0;
    buffer->words_count = words_count;
    buffer->word_length = word_length;

    return buffer;
}

void string_buffer_destroy(string_buffer* buffer) {
    pthread_mutex_destroy(&buffer->mutex);
    
    sem_destroy(&buffer->empty);
    sem_destroy(&buffer->full);

    free(buffer);
}

size_t string_buffer_write(string_buffer* buffer, const char data[]) {
    sem_wait(&buffer->empty);
    pthread_mutex_lock(&buffer->mutex);

    size_t string_size = strlen(data) + 1;
    
    if (string_size > buffer->word_length) {
        sem_post(&buffer->empty);
        pthread_mutex_unlock(&buffer->mutex);

        return 0;
    }

    memcpy(&buffer->buffer[buffer->tail * buffer->word_length], data, string_size);
    buffer->tail = (buffer->tail + 1) % buffer->words_count;

    pthread_mutex_unlock(&buffer->mutex);
    sem_post(&buffer->full);

    return string_size;
}

size_t string_buffer_read(string_buffer* buffer, char* data, size_t available_space) {
    sem_wait(&buffer->full);
    pthread_mutex_lock(&buffer->mutex);

    size_t string_size = strlen(&buffer->buffer[buffer->head * buffer->word_length]) + 1;

    if (string_size > available_space) {
        sem_post(&buffer->full);
        pthread_mutex_unlock(&buffer->mutex);
        return 0;
    }

    memcpy(data, &buffer->buffer[buffer->head * buffer->word_length], string_size);

    buffer->head = (buffer->head + 1) % buffer->words_count;

    pthread_mutex_unlock(&buffer->mutex);
    sem_post(&buffer->empty);

    return string_size;
}
