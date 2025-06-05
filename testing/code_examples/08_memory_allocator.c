#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#define POOL_SIZE (1024 * 1024)  // 1MB pool
#define ALIGNMENT 8
#define MIN_BLOCK_SIZE 16
#define MAX_BLOCKS 1000

// Block header structure
typedef struct BlockHeader {
    size_t size;
    bool is_free;
    struct BlockHeader* next;
    struct BlockHeader* prev;
} BlockHeader;

// Memory pool structure
typedef struct {
    uint8_t* memory;
    BlockHeader* free_list;
    size_t total_size;
    size_t used_size;
    size_t block_count;
} MemoryPool;

// Function to align size to ALIGNMENT
size_t align_size(size_t size) {
    return (size + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1);
}

// Function to create a new memory pool
MemoryPool* create_memory_pool(size_t size) {
    MemoryPool* pool = (MemoryPool*)malloc(sizeof(MemoryPool));
    if (!pool) {
        printf("Failed to allocate pool structure\n");
        return NULL;
    }
    
    // Align pool size
    size = align_size(size);
    
    // Allocate memory for the pool
    pool->memory = (uint8_t*)malloc(size);
    if (!pool->memory) {
        printf("Failed to allocate pool memory\n");
        free(pool);
        return NULL;
    }
    
    // Initialize pool
    pool->total_size = size;
    pool->used_size = 0;
    pool->block_count = 0;
    
    // Create initial free block
    BlockHeader* initial_block = (BlockHeader*)pool->memory;
    initial_block->size = size - sizeof(BlockHeader);
    initial_block->is_free = true;
    initial_block->next = NULL;
    initial_block->prev = NULL;
    
    pool->free_list = initial_block;
    
    return pool;
}

// Function to split a block if it's too large
void split_block(BlockHeader* block, size_t size) {
    if (block->size <= size + sizeof(BlockHeader) + MIN_BLOCK_SIZE) {
        return;  // Block is too small to split
    }
    
    // Calculate new block size
    size_t new_size = block->size - size - sizeof(BlockHeader);
    block->size = size;
    
    // Create new block
    BlockHeader* new_block = (BlockHeader*)((uint8_t*)block + sizeof(BlockHeader) + size);
    new_block->size = new_size;
    new_block->is_free = true;
    new_block->next = block->next;
    new_block->prev = block;
    
    if (block->next) {
        block->next->prev = new_block;
    }
    block->next = new_block;
}

// Function to allocate memory from the pool
void* pool_alloc(MemoryPool* pool, size_t size) {
    if (!pool || size == 0) {
        return NULL;
    }
    
    // Align requested size
    size = align_size(size);
    
    // Find suitable free block
    BlockHeader* current = pool->free_list;
    while (current) {
        if (current->is_free && current->size >= size) {
            // Split block if necessary
            split_block(current, size);
            
            // Mark block as used
            current->is_free = false;
            pool->used_size += current->size + sizeof(BlockHeader);
            pool->block_count++;
            
            // Remove from free list
            if (current->prev) {
                current->prev->next = current->next;
            } else {
                pool->free_list = current->next;
            }
            if (current->next) {
                current->next->prev = current->prev;
            }
            
            return (void*)((uint8_t*)current + sizeof(BlockHeader));
        }
        current = current->next;
    }
    
    return NULL;  // No suitable block found
}

// Function to merge adjacent free blocks
void merge_blocks(BlockHeader* block) {
    if (!block) return;
    
    // Merge with next block if it's free
    if (block->next && block->next->is_free) {
        block->size += block->next->size + sizeof(BlockHeader);
        block->next = block->next->next;
        if (block->next) {
            block->next->prev = block;
        }
    }
    
    // Merge with previous block if it's free
    if (block->prev && block->prev->is_free) {
        block->prev->size += block->size + sizeof(BlockHeader);
        block->prev->next = block->next;
        if (block->next) {
            block->next->prev = block->prev;
        }
    }
}

// Function to free memory back to the pool
void pool_free(MemoryPool* pool, void* ptr) {
    if (!pool || !ptr) {
        return;
    }
    
    // Get block header
    BlockHeader* block = (BlockHeader*)((uint8_t*)ptr - sizeof(BlockHeader));
    
    // Mark block as free
    block->is_free = true;
    pool->used_size -= block->size + sizeof(BlockHeader);
    pool->block_count--;
    
    // Add to free list
    block->next = pool->free_list;
    block->prev = NULL;
    if (pool->free_list) {
        pool->free_list->prev = block;
    }
    pool->free_list = block;
    
    // Merge adjacent free blocks
    merge_blocks(block);
}

// Function to print pool statistics
void print_pool_stats(MemoryPool* pool) {
    printf("\nMemory Pool Statistics:\n");
    printf("Total Size: %zu bytes\n", pool->total_size);
    printf("Used Size: %zu bytes\n", pool->used_size);
    printf("Free Size: %zu bytes\n", pool->total_size - pool->used_size);
    printf("Block Count: %zu\n", pool->block_count);
    
    printf("\nFree Blocks:\n");
    BlockHeader* current = pool->free_list;
    while (current) {
        printf("Block at %p: %zu bytes\n", (void*)current, current->size);
        current = current->next;
    }
}

// Function to free the memory pool
void free_memory_pool(MemoryPool* pool) {
    if (pool) {
        free(pool->memory);
        free(pool);
    }
}

int main() {
    // Create memory pool
    MemoryPool* pool = create_memory_pool(POOL_SIZE);
    if (!pool) {
        printf("Failed to create memory pool\n");
        return 1;
    }
    
    // Test allocations
    void* ptr1 = pool_alloc(pool, 100);
    void* ptr2 = pool_alloc(pool, 200);
    void* ptr3 = pool_alloc(pool, 300);
    
    printf("Allocated three blocks\n");
    print_pool_stats(pool);
    
    // Test freeing
    pool_free(pool, ptr2);
    printf("\nFreed middle block\n");
    print_pool_stats(pool);
    
    // Test allocation after free
    void* ptr4 = pool_alloc(pool, 150);
    printf("\nAllocated new block\n");
    print_pool_stats(pool);
    
    // Clean up
    pool_free(pool, ptr1);
    pool_free(pool, ptr3);
    pool_free(pool, ptr4);
    free_memory_pool(pool);
    
    return 0;
} 