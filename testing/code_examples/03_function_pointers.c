#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Define function pointer type
typedef int (*Operation)(int, int);

// Operation functions
int add(int a, int b) { return a + b; }
int subtract(int a, int b) { return a - b; }
int multiply(int a, int b) { return a * b; }
int divide(int a, int b) { 
    if (b == 0) {
        printf("Error: Division by zero\n");
        exit(1);
    }
    return a / b; 
}

// Structure to hold operation information
typedef struct {
    const char* name;
    Operation func;
} OperationInfo;

// Function to create jump table
OperationInfo* create_operation_table(int* size) {
    static OperationInfo operations[] = {
        {"add", add},
        {"subtract", subtract},
        {"multiply", multiply},
        {"divide", divide}
    };
    *size = sizeof(operations) / sizeof(operations[0]);
    return operations;
}

// Function to find operation by name
Operation find_operation(const char* name, OperationInfo* ops, int size) {
    for (int i = 0; i < size; i++) {
        if (strcmp(ops[i].name, name) == 0) {
            return ops[i].func;
        }
    }
    return NULL;
}

// Function to execute operation through function pointer
int execute_operation(Operation op, int a, int b) {
    if (op == NULL) {
        printf("Error: Invalid operation\n");
        exit(1);
    }
    return op(a, b);
}

// Function to demonstrate dynamic dispatch
void demonstrate_dynamic_dispatch(OperationInfo* ops, int size) {
    int a = 10, b = 5;
    
    printf("Demonstrating dynamic dispatch:\n");
    for (int i = 0; i < size; i++) {
        int result = execute_operation(ops[i].func, a, b);
        printf("%s(%d, %d) = %d\n", ops[i].name, a, b, result);
    }
}

// Function to demonstrate jump table
void demonstrate_jump_table() {
    // Array of function pointers
    int (*test_operations[])(int, int) = {
        add, subtract, multiply, divide
    };
    
    // Test each operation
    for (size_t i = 0; i < sizeof(test_operations) / sizeof(test_operations[0]); i++) {
        printf("Operation %zu: %d\n", i, test_operations[i](10, 5));
    }
}

int main() {
    int size;
    OperationInfo* operations = create_operation_table(&size);
    
    // Demonstrate both dynamic dispatch and jump table
    demonstrate_dynamic_dispatch(operations, size);
    demonstrate_jump_table();
    
    return 0;
} 