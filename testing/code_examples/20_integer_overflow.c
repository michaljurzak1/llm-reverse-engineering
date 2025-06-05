#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdint.h>

// Function demonstrating integer overflow in addition
void addition_overflow() {
    int max_int = INT_MAX;
    printf("Max int: %d\n", max_int);
    
    // Integer overflow in addition
    int result = max_int + 1;
    printf("Max int + 1: %d\n", result);
    
    // Check for overflow
    if (result < max_int) {
        printf("Overflow detected in addition!\n");
    }
}

// Function demonstrating integer overflow in multiplication
void multiplication_overflow() {
    int a = 1000000;
    int b = 1000000;
    
    // Integer overflow in multiplication
    int result = a * b;
    printf("1000000 * 1000000 = %d\n", result);
    
    // Check for overflow
    if (b != 0 && result / b != a) {
        printf("Overflow detected in multiplication!\n");
    }
}

// Function demonstrating size calculation vulnerability
void size_calculation_vulnerability() {
    int num_elements = 1000000;
    int element_size = 1000000;
    
    // Integer overflow in size calculation
    size_t total_size = num_elements * element_size;
    printf("Total size: %zu\n", total_size);
    
    // Allocate memory based on calculated size
    char* buffer = (char*)malloc(total_size);
    if (!buffer) {
        printf("Memory allocation failed due to size calculation error\n");
        return;
    }
    
    // Use the buffer
    memset(buffer, 'A', total_size);
    printf("Buffer allocated and initialized\n");
    
    free(buffer);
}

// Function demonstrating integer wraparound
void integer_wraparound() {
    unsigned int counter = UINT_MAX;
    printf("Max unsigned int: %u\n", counter);
    
    // Integer wraparound
    counter++;
    printf("After increment: %u\n", counter);
    
    // Check for wraparound
    if (counter == 0) {
        printf("Wraparound detected!\n");
    }
}

// Function demonstrating signed integer overflow
void signed_integer_overflow() {
    int min_int = INT_MIN;
    printf("Min int: %d\n", min_int);
    
    // Signed integer overflow
    int result = min_int - 1;
    printf("Min int - 1: %d\n", result);
    
    // Check for overflow
    if (result > min_int) {
        printf("Overflow detected in signed subtraction!\n");
    }
}

// Function demonstrating size_t overflow
void size_t_overflow() {
    size_t max_size = SIZE_MAX;
    printf("Max size_t: %zu\n", max_size);
    
    // size_t overflow
    size_t result = max_size + 1;
    printf("Max size_t + 1: %zu\n", result);
    
    // Check for overflow
    if (result < max_size) {
        printf("Overflow detected in size_t addition!\n");
    }
}

// Function demonstrating array bounds calculation
void array_bounds_calculation() {
    int array_size = 1000;
    int index = 1000000;
    
    // Integer overflow in array bounds calculation
    int* array = (int*)malloc(array_size * sizeof(int));
    if (!array) return;
    
    // Potential buffer overflow due to integer overflow
    if (index < array_size) {  // This check might fail due to integer overflow
        array[index] = 42;
        printf("Array element set\n");
    }
    
    free(array);
}

// Function demonstrating loop counter overflow
void loop_counter_overflow() {
    unsigned int counter = 0;
    unsigned int target = UINT_MAX - 5;
    
    // Loop that might never terminate due to integer overflow
    while (counter < target) {
        counter++;
        if (counter == 0) {
            printf("Loop counter overflow detected!\n");
            break;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Usage: %s <test_number>\n", argv[0]);
        printf("1: Addition overflow\n");
        printf("2: Multiplication overflow\n");
        printf("3: Size calculation vulnerability\n");
        printf("4: Integer wraparound\n");
        printf("5: Signed integer overflow\n");
        printf("6: size_t overflow\n");
        printf("7: Array bounds calculation\n");
        printf("8: Loop counter overflow\n");
        return 1;
    }
    
    int test = atoi(argv[1]);
    
    switch (test) {
        case 1:
            addition_overflow();
            break;
        case 2:
            multiplication_overflow();
            break;
        case 3:
            size_calculation_vulnerability();
            break;
        case 4:
            integer_wraparound();
            break;
        case 5:
            signed_integer_overflow();
            break;
        case 6:
            size_t_overflow();
            break;
        case 7:
            array_bounds_calculation();
            break;
        case 8:
            loop_counter_overflow();
            break;
        default:
            printf("Invalid test number\n");
            return 1;
    }
    
    return 0;
} 