#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define BUFFER_SIZE 5
#define NUM_PRODUCERS 2
#define NUM_CONSUMERS 2
#define NUM_ITEMS 10

// Buffer structure
typedef struct {
    int* items;
    int in;
    int out;
    int count;
    pthread_mutex_t mutex;
    pthread_cond_t not_full;
    pthread_cond_t not_empty;
} Buffer;

// Thread arguments structure
typedef struct {
    int id;
    Buffer* buffer;
    int num_items;
} ThreadArgs;

// Function to initialize buffer
Buffer* create_buffer(int size) {
    Buffer* buffer = (Buffer*)malloc(sizeof(Buffer));
    if (!buffer) {
        printf("Failed to allocate buffer structure\n");
        return NULL;
    }
    
    buffer->items = (int*)malloc(size * sizeof(int));
    if (!buffer->items) {
        printf("Failed to allocate buffer memory\n");
        free(buffer);
        return NULL;
    }
    
    buffer->in = 0;
    buffer->out = 0;
    buffer->count = 0;
    
    pthread_mutex_init(&buffer->mutex, NULL);
    pthread_cond_init(&buffer->not_full, NULL);
    pthread_cond_init(&buffer->not_empty, NULL);
    
    return buffer;
}

// Function to free buffer
void free_buffer(Buffer* buffer) {
    if (buffer) {
        pthread_mutex_destroy(&buffer->mutex);
        pthread_cond_destroy(&buffer->not_full);
        pthread_cond_destroy(&buffer->not_empty);
        free(buffer->items);
        free(buffer);
    }
}

// Producer function
void* producer(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    Buffer* buffer = args->buffer;
    int id = args->id;
    int item;
    
    for (int i = 0; i < args->num_items; i++) {
        // Generate item
        item = rand() % 100;
        
        // Lock mutex
        pthread_mutex_lock(&buffer->mutex);
        
        // Wait if buffer is full
        while (buffer->count == BUFFER_SIZE) {
            printf("Producer %d: Buffer full, waiting...\n", id);
            pthread_cond_wait(&buffer->not_full, &buffer->mutex);
        }
        
        // Add item to buffer
        buffer->items[buffer->in] = item;
        buffer->in = (buffer->in + 1) % BUFFER_SIZE;
        buffer->count++;
        
        printf("Producer %d: Produced item %d\n", id, item);
        
        // Signal that buffer is not empty
        pthread_cond_signal(&buffer->not_empty);
        
        // Unlock mutex
        pthread_mutex_unlock(&buffer->mutex);
        
        // Simulate production time
        usleep(rand() % 100000);
    }
    
    return NULL;
}

// Consumer function
void* consumer(void* arg) {
    ThreadArgs* args = (ThreadArgs*)arg;
    Buffer* buffer = args->buffer;
    int id = args->id;
    int item;
    
    for (int i = 0; i < args->num_items; i++) {
        // Lock mutex
        pthread_mutex_lock(&buffer->mutex);
        
        // Wait if buffer is empty
        while (buffer->count == 0) {
            printf("Consumer %d: Buffer empty, waiting...\n", id);
            pthread_cond_wait(&buffer->not_empty, &buffer->mutex);
        }
        
        // Remove item from buffer
        item = buffer->items[buffer->out];
        buffer->out = (buffer->out + 1) % BUFFER_SIZE;
        buffer->count--;
        
        printf("Consumer %d: Consumed item %d\n", id, item);
        
        // Signal that buffer is not full
        pthread_cond_signal(&buffer->not_full);
        
        // Unlock mutex
        pthread_mutex_unlock(&buffer->mutex);
        
        // Simulate consumption time
        usleep(rand() % 100000);
    }
    
    return NULL;
}

int main() {
    // Initialize random seed
    srand(time(NULL));
    
    // Create buffer
    Buffer* buffer = create_buffer(BUFFER_SIZE);
    if (!buffer) {
        printf("Failed to create buffer\n");
        return 1;
    }
    
    // Create thread arrays
    pthread_t producers[NUM_PRODUCERS];
    pthread_t consumers[NUM_CONSUMERS];
    ThreadArgs producer_args[NUM_PRODUCERS];
    ThreadArgs consumer_args[NUM_CONSUMERS];
    
    // Initialize producer threads
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        producer_args[i].id = i;
        producer_args[i].buffer = buffer;
        producer_args[i].num_items = NUM_ITEMS;
        pthread_create(&producers[i], NULL, producer, &producer_args[i]);
    }
    
    // Initialize consumer threads
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        consumer_args[i].id = i;
        consumer_args[i].buffer = buffer;
        consumer_args[i].num_items = NUM_ITEMS;
        pthread_create(&consumers[i], NULL, consumer, &consumer_args[i]);
    }
    
    // Wait for producer threads to complete
    for (int i = 0; i < NUM_PRODUCERS; i++) {
        pthread_join(producers[i], NULL);
    }
    
    // Wait for consumer threads to complete
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        pthread_join(consumers[i], NULL);
    }
    
    // Clean up
    free_buffer(buffer);
    
    return 0;
} 