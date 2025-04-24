#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Prompt the user for their name and strip the trailing newline */
void get_user_name(char *name) {
    printf("Please enter your name: ");
    if (fgets(name, 50, stdin)) {
        name[strcspn(name, "\n")] = '\0';
    }
}

/* Prompt with the user's name and read an integer from stdin */
int get_number(const char *name) {
    int n = 0;
    printf("Hi %s! Enter a number to calculate its factorial: ", name);
    if (scanf("%d", &n) != 1) {
        /* Invalid or no input: default to 0 */
        n = 0;
    }
    return n;
}

/* Recursively compute factorial */
int factorial(int n) {
    if (n <= 1) {
        return 1;
    }
    return n * factorial(n - 1);
}

/* Validate & compute factorial, or print error if negative */
void calculate_factorial(int n) {
    if (n < 0) {
        puts("Sorry, factorial is not defined for negative numbers.");
        return;
    }
    int result = factorial(n);
    printf("The factorial of %d is %d\n", n, result);
}

/* Print a simple star pattern of height up to 5 (or n if smaller) */
void print_pattern(int n) {
    puts("\nHere's a small pattern for you:");
    int limit = (n < 5) ? n : 5;
    for (int i = 0; i < limit; i++) {
        for (int j = 0; j <= i; j++) {
            putchar('*');
        }
        putchar('\n');
    }
}

int main(int argc, char **argv, char **envp) {
    char username[50];
    int number;

    puts("Welcome to the Demo Program!");
    get_user_name(username);
    number = get_number(username);
    calculate_factorial(number);
    print_pattern(number);

    return 0;
}