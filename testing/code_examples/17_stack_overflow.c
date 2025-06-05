#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 16
#define PASSWORD "secret123"

// Vulnerable function with buffer overflow
void vulnerable_function(char* input) {
    char buffer[BUFFER_SIZE];
    int is_admin = 0;
    
    // Unsafe string copy - vulnerable to buffer overflow
    strcpy(buffer, input);
    
    // This check can be bypassed by buffer overflow
    if (is_admin) {
        printf("Access granted! You are an admin.\n");
        // Simulate admin actions
        printf("Performing admin operations...\n");
    } else {
        printf("Access denied. You are not an admin.\n");
    }
}

// Function demonstrating buffer overflow vulnerability
void gets_vulnerable() {
    char buffer[10];
    printf("Enter a string: ");
    fgets(buffer, sizeof(buffer), stdin);  // Use fgets instead of gets
    printf("You entered: %s\n", buffer);
}

// Function with sprintf vulnerability
void sprintf_vulnerable(const char* format, const char* input) {
    char buffer[BUFFER_SIZE];
    
    // Unsafe sprintf - vulnerable to buffer overflow
    sprintf(buffer, format, input);
    
    printf("Formatted string: %s\n", buffer);
}

// Function with strcat vulnerability
void strcat_vulnerable(const char* input) {
    char buffer[BUFFER_SIZE] = "Hello, ";
    
    // Unsafe strcat - vulnerable to buffer overflow
    strcat(buffer, input);
    
    printf("Concatenated string: %s\n", buffer);
}

// Function demonstrating stack frame
void stack_frame_demo() {
    int local_var = 42;
    char buffer[10];
    
    // Compare addresses using proper casting
    printf("Stack grows: %s\n", 
           (void*)&local_var > (void*)buffer ? "downward" : "upward");
}

// Function to demonstrate buffer boundary detection
void boundary_demo() {
    char buffer1[BUFFER_SIZE];
    char buffer2[BUFFER_SIZE];
    int canary = 0xDEADBEEF;
    
    printf("Buffer1 address: %p\n", (void*)buffer1);
    printf("Buffer2 address: %p\n", (void*)buffer2);
    printf("Canary address: %p\n", (void*)&canary);
    
    // Demonstrate buffer overflow detection
    printf("Canary value before: 0x%X\n", canary);
    
    // This would cause a buffer overflow
    // strcpy(buffer1, "This string is too long for the buffer");
    
    printf("Canary value after: 0x%X\n", canary);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <test_number>\n", argv[0]);
        printf("1: strcpy vulnerability\n");
        printf("2: gets() vulnerability\n");
        printf("3: sprintf vulnerability\n");
        printf("4: strcat vulnerability\n");
        printf("5: stack frame layout\n");
        printf("6: buffer boundary detection\n");
        return 1;
    }
    
    int test = atoi(argv[1]);
    
    switch (test) {
        case 1:
            // Test strcpy vulnerability
            if (argc < 3) {
                printf("Please provide input string for strcpy test\n");
                return 1;
            }
            vulnerable_function(argv[2]);
            break;
            
        case 2:
            // Test gets() vulnerability
            gets_vulnerable();
            break;
            
        case 3:
            // Test sprintf vulnerability
            if (argc < 3) {
                printf("Please provide input string for sprintf test\n");
                return 1;
            }
            sprintf_vulnerable("%s", argv[2]);
            break;
            
        case 4:
            // Test strcat vulnerability
            if (argc < 3) {
                printf("Please provide input string for strcat test\n");
                return 1;
            }
            strcat_vulnerable(argv[2]);
            break;
            
        case 5:
            // Demonstrate stack frame layout
            stack_frame_demo();
            break;
            
        case 6:
            // Demonstrate buffer boundary detection
            boundary_demo();
            break;
            
        default:
            printf("Invalid test number\n");
            return 1;
    }
    
    return 0;
} 