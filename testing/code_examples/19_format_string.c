#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

// Global variables to demonstrate memory leaks
int secret_value = 0x12345678;
char* secret_string = "This is a secret string";

// Safe version using format string
void safe_printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

// Function with format string vulnerability
void vulnerable_printf(const char* user_input) {
    // Use safe version instead
    safe_printf("%s", user_input);
}

// Function with format string vulnerability and memory leak
void vulnerable_printf_with_memory(const char* user_input) {
    char buffer[100];
    strncpy(buffer, user_input, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';
    safe_printf("%s", buffer);
}

// Function demonstrating format string attack on stack
void format_string_stack_attack(const char* user_input) {
    int local_var = 0xABCD;
    char buffer[100];
    
    // Copy user input to buffer
    strcpy(buffer, user_input);
    
    // Vulnerable printf
    printf(buffer);
    printf("\n");
    
    // Local variable might be modified by format string attack
    printf("Local variable value: 0x%X\n", local_var);
}

// Function demonstrating format string attack on heap
void format_string_heap_attack(const char* user_input) {
    char* heap_buffer = (char*)malloc(100);
    if (!heap_buffer) return;
    
    // Copy user input to heap buffer
    strcpy(heap_buffer, user_input);
    
    // Vulnerable printf
    printf(heap_buffer);
    printf("\n");
    
    // Heap buffer might be modified by format string attack
    printf("Heap buffer: %s\n", heap_buffer);
    
    free(heap_buffer);
}

// Function demonstrating format string attack on global variables
void format_string_global_attack(const char* user_input) {
    // Vulnerable printf
    printf(user_input);
    printf("\n");
    
    // Global variables might be modified by format string attack
    printf("Secret value: 0x%X\n", secret_value);
    printf("Secret string: %s\n", secret_string);
}

// Function demonstrating format string attack with multiple arguments
void format_string_multiple_args(const char* user_input) {
    int arg1 = 0x1111;
    int arg2 = 0x2222;
    int arg3 = 0x3333;
    
    // Vulnerable printf with multiple arguments
    printf(user_input, arg1, arg2, arg3);
    printf("\n");
    
    // Arguments might be modified by format string attack
    printf("Arguments: 0x%X, 0x%X, 0x%X\n", arg1, arg2, arg3);
}

// Function demonstrating format string attack with string manipulation
void format_string_string_manipulation(const char* user_input) {
    char buffer[100];
    
    // Copy user input to buffer
    strcpy(buffer, user_input);
    
    // Vulnerable printf
    printf(buffer);
    printf("\n");
    
    // Buffer might be modified by format string attack
    printf("Buffer content: %s\n", buffer);
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printf("Usage: %s <test_number> <format_string>\n", argv[0]);
        printf("1: Basic format string vulnerability\n");
        printf("2: Format string with memory leak\n");
        printf("3: Format string stack attack\n");
        printf("4: Format string heap attack\n");
        printf("5: Format string global attack\n");
        printf("6: Format string multiple arguments\n");
        printf("7: Format string string manipulation\n");
        return 1;
    }
    
    int test = atoi(argv[1]);
    const char* format_string = argv[2];
    
    switch (test) {
        case 1:
            vulnerable_printf(format_string);
            break;
        case 2:
            vulnerable_printf_with_memory(format_string);
            break;
        case 3:
            format_string_stack_attack(format_string);
            break;
        case 4:
            format_string_heap_attack(format_string);
            break;
        case 5:
            format_string_global_attack(format_string);
            break;
        case 6:
            format_string_multiple_args(format_string);
            break;
        case 7:
            format_string_string_manipulation(format_string);
            break;
        default:
            printf("Invalid test number\n");
            return 1;
    }
    
    return 0;
} 