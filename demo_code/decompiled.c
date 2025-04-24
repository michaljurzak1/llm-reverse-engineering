#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*
 * Compute the factorial of a non-negative integer n.
 * For n <= 1, returns 1. For n > 1, returns n * factorial(n-1).
 */
long factorial(int n) {
    if (n <= 1) {
        return 1;
    }
    return (long)n * factorial(n - 1);
}

int main(void) {
    char name[50];
    int num;

    /* Welcome message */
    puts("Welcome to the Demo Program!");

    /* Prompt for user name */
    printf("Please enter your name: ");
    if (fgets(name, sizeof(name), stdin) == NULL) {
        /* Input error—exit */
        return EXIT_FAILURE;
    }
    /* Strip trailing newline if present */
    name[strcspn(name, "\n")] = '\0';

    /* Ask for a number to compute factorial */
    printf("Hi %s! Enter a number to calculate its factorial: ", name);
    if (scanf("%d", &num) != 1) {
        /* Failed to parse integer */
        return EXIT_FAILURE;
    }

    /* Handle negative input */
    if (num < 0) {
        puts("Sorry, factorial is not defined for negative numbers.");
    } else {
        long result = factorial(num);
        printf("The factorial of %d is %ld\n", num, result);
    }

    /* Print a small star pattern up to 5 rows or num rows, whichever is smaller */
    puts("\nHere's a small pattern for you:");
    for (int i = 0; i < num && i <= 4; i++) {
        for (int j = 0; j <= i; j++) {
            printf("* ");
        }
        putchar('\n');
    }

    return 0;
}