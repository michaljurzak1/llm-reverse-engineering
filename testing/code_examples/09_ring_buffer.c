#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define RING_BUFFER_SIZE 8

// Ring buffer structure
typedef struct {
    int* buffer;
    size_t size;
    size_t head;
    size_t tail;
    size_t count;
    bool is_full;
} RingBuffer;

// Function to create a new ring buffer
RingBuffer* create_ring_buffer(size_t size) {
    RingBuffer* rb = (RingBuffer*)malloc(sizeof(RingBuffer));
    if (!rb) {
        printf("Failed to allocate ring buffer structure\n");
        return NULL;
    }
    
    rb->buffer = (int*)malloc(size * sizeof(int));
    if (!rb->buffer) {
        printf("Failed to allocate ring buffer memory\n");
        free(rb);
        return NULL;
    }
    
    rb->size = size;
    rb->head = 0;
    rb->tail = 0;
    rb->count = 0;
    rb->is_full = false;
    
    return rb;
}

// Function to check if buffer is empty
bool is_empty(RingBuffer* rb) {
    return !rb->is_full && rb->head == rb->tail;
}

// Function to check if buffer is full
bool is_full(RingBuffer* rb) {
    return rb->is_full;
}

// Function to get current count
size_t get_count(RingBuffer* rb) {
    return rb->count;
}

// Function to get available space
size_t get_available(RingBuffer* rb) {
    return rb->size - rb->count;
}

// Function to enqueue an element
bool enqueue(RingBuffer* rb, int value) {
    if (is_full(rb)) {
        printf("Buffer is full\n");
        return false;
    }
    
    rb->buffer[rb->tail] = value;
    rb->tail = (rb->tail + 1) % rb->size;
    rb->count++;
    
    if (rb->tail == rb->head) {
        rb->is_full = true;
    }
    
    return true;
}

// Function to dequeue an element
bool dequeue(RingBuffer* rb, int* value) {
    if (is_empty(rb)) {
        printf("Buffer is empty\n");
        return false;
    }
    
    *value = rb->buffer[rb->head];
    rb->head = (rb->head + 1) % rb->size;
    rb->count--;
    rb->is_full = false;
    
    return true;
}

// Function to peek at the next element
bool peek(RingBuffer* rb, int* value) {
    if (is_empty(rb)) {
        printf("Buffer is empty\n");
        return false;
    }
    
    *value = rb->buffer[rb->head];
    return true;
}

// Function to clear the buffer
void clear(RingBuffer* rb) {
    rb->head = 0;
    rb->tail = 0;
    rb->count = 0;
    rb->is_full = false;
}

// Function to print buffer contents
void print_buffer(RingBuffer* rb) {
    printf("\nRing Buffer Contents:\n");
    printf("Size: %zu, Count: %zu, Head: %zu, Tail: %zu\n",
           rb->size, rb->count, rb->head, rb->tail);
    
    if (is_empty(rb)) {
        printf("Buffer is empty\n");
        return;
    }
    
    printf("Elements: ");
    size_t current = rb->head;
    size_t count = 0;
    
    while (count < rb->count) {
        printf("%d ", rb->buffer[current]);
        current = (current + 1) % rb->size;
        count++;
    }
    printf("\n");
}

// Function to free the ring buffer
void free_ring_buffer(RingBuffer* rb) {
    if (rb) {
        free(rb->buffer);
        free(rb);
    }
}

int main() {
    // Create ring buffer
    RingBuffer* rb = create_ring_buffer(RING_BUFFER_SIZE);
    if (!rb) {
        printf("Failed to create ring buffer\n");
        return 1;
    }
    
    // Test enqueue operations
    printf("Testing enqueue operations:\n");
    for (int i = 1; i <= 10; i++) {
        if (enqueue(rb, i)) {
            printf("Enqueued: %d\n", i);
        }
    }
    print_buffer(rb);
    
    // Test dequeue operations
    printf("\nTesting dequeue operations:\n");
    int value;
    for (int i = 0; i < 3; i++) {
        if (dequeue(rb, &value)) {
            printf("Dequeued: %d\n", value);
        }
    }
    print_buffer(rb);
    
    // Test enqueue after dequeue
    printf("\nTesting enqueue after dequeue:\n");
    for (int i = 11; i <= 13; i++) {
        if (enqueue(rb, i)) {
            printf("Enqueued: %d\n", i);
        }
    }
    print_buffer(rb);
    
    // Test peek operation
    printf("\nTesting peek operation:\n");
    if (peek(rb, &value)) {
        printf("Peeked value: %d\n", value);
    }
    
    // Test clear operation
    printf("\nTesting clear operation:\n");
    clear(rb);
    print_buffer(rb);
    
    // Clean up
    free_ring_buffer(rb);
    
    return 0;
} 