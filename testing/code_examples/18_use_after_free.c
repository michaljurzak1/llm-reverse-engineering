#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Structure to demonstrate use-after-free
typedef struct {
    char* data;
    int size;
    void (*callback)(void*);
} Resource;

// Callback function type
typedef void (*CallbackFunc)(void*);

// Global pointer to track freed resources
Resource* global_resource = NULL;

// Function to create a new resource
Resource* create_resource(const char* data, int size, CallbackFunc callback) {
    Resource* res = (Resource*)malloc(sizeof(Resource));
    if (!res) return NULL;
    
    res->data = (char*)malloc(size);
    if (!res->data) {
        free(res);
        return NULL;
    }
    
    memcpy(res->data, data, size);
    res->size = size;
    res->callback = callback;
    
    return res;
}

// Function to free a resource
void free_resource(Resource* res) {
    if (!res) return;
    
    if (res->data) {
        free(res->data);
        res->data = NULL;  // Clear pointer after free
    }
    
    free(res);
}

// Function demonstrating use-after-free
void use_after_free_example() {
    Resource* res = create_resource("Hello", 6, NULL);
    if (!res) return;
    
    // Store pointer in global variable
    global_resource = res;
    
    // Free the resource
    free_resource(res);
    
    // Use after free - accessing freed memory
    printf("Use after free: %s\n", global_resource->data);  // Dangerous!
    
    // Double free attempt
    free_resource(global_resource);  // Dangerous!
    
    global_resource = NULL;
}

// Function demonstrating dangling pointer
void dangling_pointer_example() {
    Resource* res = create_resource("Test", 5, NULL);
    if (!res) return;
    
    Resource* alias = res;  // Create an alias pointer
    
    // Free the original resource
    free_resource(res);
    
    // Use the dangling pointer
    printf("Dangling pointer: %s\n", alias->data);  // Dangerous!
    
    // Try to modify through dangling pointer
    alias->size = 100;  // Dangerous!
}

// Function demonstrating memory state across calls
void memory_state_example() {
    Resource* res = create_resource("State", 6, NULL);
    if (!res) return;
    
    // Store pointer in global variable
    global_resource = res;
    
    // Free the resource
    free_resource(res);
    
    // Allocate new memory that might reuse the freed space
    char* new_data = (char*)malloc(6);
    if (new_data) {
        strcpy(new_data, "New");
        
        // Access through global pointer (use-after-free)
        printf("Memory state: %s\n", global_resource->data);  // Dangerous!
        
        free(new_data);
    }
    
    global_resource = NULL;
}

// Function demonstrating callback after free
void callback_after_free_example() {
    Resource* res = create_resource("Callback", 9, NULL);
    if (!res) return;
    
    // Set up callback
    res->callback = (CallbackFunc)printf;
    
    // Free the resource
    free_resource(res);
    
    // Try to use callback after free
    if (res->callback) {
        res->callback("Callback after free\n");  // Dangerous!
    }
}

// Function to demonstrate memory reuse
void memory_reuse_example() {
    Resource* res1 = create_resource("First", 6, NULL);
    if (!res1) return;
    
    // Free first resource
    free_resource(res1);
    
    // Create second resource that might reuse the same memory
    Resource* res2 = create_resource("Second", 7, NULL);
    if (!res2) return;
    
    // Use-after-free with first resource
    printf("Memory reuse: %s\n", res1->data);  // Dangerous!
    
    free_resource(res2);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <test_number>\n", argv[0]);
        printf("1: Use after free\n");
        printf("2: Dangling pointer\n");
        printf("3: Memory state across calls\n");
        printf("4: Callback after free\n");
        printf("5: Memory reuse\n");
        return 1;
    }
    
    int test = atoi(argv[1]);
    
    switch (test) {
        case 1:
            use_after_free_example();
            break;
        case 2:
            dangling_pointer_example();
            break;
        case 3:
            memory_state_example();
            break;
        case 4:
            callback_after_free_example();
            break;
        case 5:
            memory_reuse_example();
            break;
        default:
            printf("Invalid test number\n");
            return 1;
    }
    
    return 0;
} 