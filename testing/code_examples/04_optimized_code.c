#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define SIZE 4

// Inline function with register optimization
static inline int fast_multiply(int a, int b) {
    register int result = 0;
    while (b > 0) {
        if (b & 1) result += a;
        a <<= 1;
        b >>= 1;
    }
    return result;
}

// Loop unrolling example
void unrolled_memset(void* dest, int val, size_t count) {
    unsigned char* ptr = (unsigned char*)dest;
    size_t i;
    
    // Unroll by 8
    for (i = 0; i < count - 7; i += 8) {
        ptr[i] = val;
        ptr[i + 1] = val;
        ptr[i + 2] = val;
        ptr[i + 3] = val;
        ptr[i + 4] = val;
        ptr[i + 5] = val;
        ptr[i + 6] = val;
        ptr[i + 7] = val;
    }
    
    // Handle remaining elements
    for (; i < count; i++) {
        ptr[i] = val;
    }
}

// Dead code elimination example
int complex_calculation(int x) {
    int result = 0;
    
    // This branch will be eliminated by optimizer
    if (x < 0) {
        result = -x;
    } else {
        result = x;
    }
    
    // This calculation will be optimized away
    int temp = result * 2;
    temp = temp / 2;
    
    return temp;
}

// Strength reduction example
void matrix_multiply(int* result, const int* a, const int* b, int size) {
    register int i, j, k;
    register int sum;
    
    for (i = 0; i < size; i++) {
        for (j = 0; j < size; j++) {
            sum = 0;
            // Strength reduction: multiplication replaced with addition
            for (k = 0; k < size; k++) {
                sum += a[i * size + k] * b[k * size + j];
            }
            result[i * size + j] = sum;
        }
    }
}

// Common subexpression elimination example
void process_array(int* arr, int size) {
    register int i;
    register int temp;
    
    for (i = 0; i < size; i++) {
        // Common subexpression: arr[i] * arr[i] calculated once
        temp = arr[i] * arr[i];
        arr[i] = temp + temp;  // Reuse temp instead of calculating again
    }
}

// Function inlining and constant folding example
static inline int optimized_calc(int x) {
    const int multiplier = 2;
    const int adder = 5;
    
    // Constant folding: 2 * 5 = 10
    return x * multiplier + adder;
}

int main() {
    // Initialize matrices with static arrays
    int matrix_a[SIZE * SIZE];
    int matrix_b[SIZE * SIZE];
    
    // Initialize matrix_a
    for (int i = 0; i < SIZE * SIZE; i++) {
        matrix_a[i] = i + 1;
    }
    
    // Initialize matrix_b (identity matrix)
    for (int i = 0; i < SIZE * SIZE; i++) {
        matrix_b[i] = (i % (SIZE + 1) == 0) ? 1 : 0;
    }
    
    int result[SIZE * SIZE];
    
    // Test fast multiplication
    printf("Fast multiply 7 * 8 = %d\n", fast_multiply(7, 8));
    
    // Test unrolled memset
    char buffer[100];
    unrolled_memset(buffer, 0xFF, sizeof(buffer));
    printf("First byte of buffer: 0x%02X\n", (unsigned char)buffer[0]);
    
    // Test complex calculation (dead code elimination)
    printf("Complex calculation(5) = %d\n", complex_calculation(5));
    
    // Test matrix multiplication
    matrix_multiply(result, matrix_a, matrix_b, SIZE);
    printf("Matrix multiplication result[0] = %d\n", result[0]);
    
    // Test array processing
    int arr[] = {1, 2, 3, 4, 5};
    process_array(arr, 5);
    printf("Processed array[0] = %d\n", arr[0]);
    
    // Test optimized calculation
    printf("Optimized calc(10) = %d\n", optimized_calc(10));
    
    return 0;
} 