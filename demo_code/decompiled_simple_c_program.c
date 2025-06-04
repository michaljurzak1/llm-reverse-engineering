#include <stdio.h>
#include <string.h>

// Compute factorial recursively
// Returns 1 for n <= 1, otherwise n * factorial(n - 1)
static int factorial(int n) {
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n - 1);
}

int main(void) {
    char name[50];
    int num;
    int result;

    puts("Welcome to the Demo Program!");
    printf("Please enter your name: ");
    if (fgets(name, sizeof(name), stdin) != NULL) {
        // Remove trailing newline, if any
        name[strcspn(name, "\n")] = '\0';
    }

    printf("Hi %s! Enter a number to calculate its factorial: ", name);
    if (scanf("%d", &num) != 1) {
        // Invalid input
        return 1;
    }

    if (num < 0) {
        puts("Sorry, factorial is not defined for negative numbers.");
    } else {
        result = factorial(num);
        printf("The factorial of %d is %d\n", num, result);
    }

    puts("\nHere's a small pattern for you:");

    // Print a simple star-triangle pattern.
    // For row = 0..min(num-1, 4), print row+1 stars.
    for (int row = 0; row < num && row <= 4; row++) {
        for (int col = 0; col <= row; col++) {
            printf("* ");
        }
        putchar('\n');
    }

    return 0;
}