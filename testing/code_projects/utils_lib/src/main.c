#include "utils.h"
#include <stdio.h>

int main() {
    int arr[] = {1, 2, 3, 4, 5};
    printf("Factorial of 5: %d\n", factorial(5));
    printf("Fibonacci of 10: %d\n", fibonacci(10));
    print_array(arr, 5);
    return 0;
} 