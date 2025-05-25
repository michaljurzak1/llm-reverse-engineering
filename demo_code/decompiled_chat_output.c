#include <stdio.h>
#include <string.h>
#include <stdint.h>

/*
 * Compute factorial recursively.
 * For n <= 1 returns 1, otherwise n * factorial(n-1).
 */
static int64_t factorial(int64_t n) {
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n - 1);
}

int main(int argc, char **argv) {
    char name[50];
    int num;

    /* Greet the user */
    puts("Welcome to the Demo Program!");
    printf("Please enter your name: ");

    /* Read a line safely into 'name' and strip newline */
    if (fgets(name, sizeof(name), stdin) == NULL) {
        return 1;
    }
    name[strcspn(name, "\n")] = '\0';

    /* Prompt for a number */
    printf("Hi %s! Enter a number to calculate its factorial: ", name);
    if (scanf("%d", &num) != 1) {
        return 1;
    }

    /* Handle negative input */
    if (num < 0) {
        puts("Sorry, factorial is not defined for negative numbers.");
    } else {
        /* Compute and display factorial */
        int64_t result = factorial(num);
        printf("The factorial of %d is %lld\n", num, result);

        /* Print a simple right-angled triangular pattern of '*' */
        puts("\nHere's a small pattern for you:");
        for (int row = 0; row < num; ++row) {
            for (int col = 0; col <= row; ++col) {
                printf("* ");
            }
            putchar('\n');
        }
    }

    return 0;
}