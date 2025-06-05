#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_SIZE 16
#define LOAD_FACTOR 0.75
#define GROWTH_FACTOR 2

// Node structure for chaining
typedef struct HashNode {
    char* key;
    int value;
    struct HashNode* next;
} HashNode;

// Hash table structure
typedef struct {
    HashNode** table;
    size_t size;
    size_t capacity;
    size_t collisions;
} HashTable;

// Function to create a new hash node
HashNode* create_node(const char* key, int value) {
    HashNode* node = (HashNode*)malloc(sizeof(HashNode));
    if (!node) {
        printf("Memory allocation failed\n");
        exit(1);
    }
    
    node->key = strdup(key);
    if (!node->key) {
        printf("Memory allocation failed\n");
        free(node);
        exit(1);
    }
    
    node->value = value;
    node->next = NULL;
    return node;
}

// Function to create a new hash table
HashTable* create_hash_table() {
    HashTable* table = (HashTable*)malloc(sizeof(HashTable));
    if (!table) {
        printf("Memory allocation failed\n");
        exit(1);
    }
    
    table->capacity = INITIAL_SIZE;
    table->size = 0;
    table->collisions = 0;
    
    table->table = (HashNode**)calloc(table->capacity, sizeof(HashNode*));
    if (!table->table) {
        printf("Memory allocation failed\n");
        free(table);
        exit(1);
    }
    
    return table;
}

// Hash function
size_t hash_function(const char* key, size_t capacity) {
    size_t hash = 5381;
    int c;
    
    while ((c = *key++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    
    return hash % capacity;
}

// Function to resize the hash table
void resize_table(HashTable* table) {
    size_t old_capacity = table->capacity;
    HashNode** old_table = table->table;
    
    // Create new table with increased capacity
    table->capacity *= GROWTH_FACTOR;
    table->table = (HashNode**)calloc(table->capacity, sizeof(HashNode*));
    if (!table->table) {
        printf("Memory allocation failed\n");
        exit(1);
    }
    
    // Rehash all elements
    for (size_t i = 0; i < old_capacity; i++) {
        HashNode* current = old_table[i];
        while (current) {
            HashNode* next = current->next;
            size_t new_index = hash_function(current->key, table->capacity);
            
            current->next = table->table[new_index];
            table->table[new_index] = current;
            
            current = next;
        }
    }
    
    free(old_table);
}

// Function to insert a key-value pair
void insert(HashTable* table, const char* key, int value) {
    // Check if resize is needed
    if ((double)table->size / table->capacity >= LOAD_FACTOR) {
        resize_table(table);
    }
    
    size_t index = hash_function(key, table->capacity);
    HashNode* current = table->table[index];
    
    // Check if key already exists
    while (current) {
        if (strcmp(current->key, key) == 0) {
            current->value = value;
            return;
        }
        current = current->next;
    }
    
    // Create new node
    HashNode* new_node = create_node(key, value);
    
    // Check for collision
    if (table->table[index]) {
        table->collisions++;
    }
    
    // Insert at the beginning of the chain
    new_node->next = table->table[index];
    table->table[index] = new_node;
    table->size++;
}

// Function to get a value by key
int get(HashTable* table, const char* key) {
    size_t index = hash_function(key, table->capacity);
    HashNode* current = table->table[index];
    
    while (current) {
        if (strcmp(current->key, key) == 0) {
            return current->value;
        }
        current = current->next;
    }
    
    return -1; // Key not found
}

// Function to remove a key-value pair
void remove_key(HashTable* table, const char* key) {
    size_t index = hash_function(key, table->capacity);
    HashNode* current = table->table[index];
    HashNode* prev = NULL;
    
    while (current) {
        if (strcmp(current->key, key) == 0) {
            if (prev) {
                prev->next = current->next;
            } else {
                table->table[index] = current->next;
            }
            
            free(current->key);
            free(current);
            table->size--;
            return;
        }
        prev = current;
        current = current->next;
    }
}

// Function to print the hash table
void print_table(HashTable* table) {
    printf("\nHash Table Contents:\n");
    for (size_t i = 0; i < table->capacity; i++) {
        printf("Bucket %zu: ", i);
        HashNode* current = table->table[i];
        while (current) {
            printf("[%s: %d] -> ", current->key, current->value);
            current = current->next;
        }
        printf("NULL\n");
    }
    printf("\nTotal collisions: %zu\n", table->collisions);
}

// Function to free the hash table
void free_table(HashTable* table) {
    for (size_t i = 0; i < table->capacity; i++) {
        HashNode* current = table->table[i];
        while (current) {
            HashNode* next = current->next;
            free(current->key);
            free(current);
            current = next;
        }
    }
    free(table->table);
    free(table);
}

int main() {
    HashTable* table = create_hash_table();
    
    // Insert some key-value pairs
    insert(table, "apple", 1);
    insert(table, "banana", 2);
    insert(table, "cherry", 3);
    insert(table, "date", 4);
    insert(table, "elderberry", 5);
    
    // Test collision handling
    insert(table, "apple", 10); // Should update existing value
    
    // Print the table
    print_table(table);
    
    // Test get operation
    printf("\nValue for 'banana': %d\n", get(table, "banana"));
    printf("Value for 'fig': %d\n", get(table, "fig")); // Should return -1
    
    // Test remove operation
    printf("\nRemoving 'cherry'...\n");
    remove_key(table, "cherry");
    print_table(table);
    
    // Clean up
    free_table(table);
    
    return 0;
} 