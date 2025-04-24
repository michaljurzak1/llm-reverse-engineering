#include <stdio.h>
#include <string.h>

#define MAX_NAME 50

int factorial(int n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}

int main() {
    char name[MAX_NAME];
    int number;

    printf("Welcome to the Demo Program!\n");
    printf("Please enter your name: ");
    fgets(name, MAX_NAME, stdin);
    name[strcspn(name, "\n")] = 0; // Remove trailing newline

    printf("Hi %s! Enter a number to calculate its factorial: ", name);
    scanf("%d", &number);

    if (number < 0) {
        printf("Sorry, factorial is not defined for negative numbers.\n");
    } else {
        printf("The factorial of %d is %d\n", number, factorial(number));
    }

    printf("\nHere's a small pattern for you:\n");
    for (int i = 0; i < number && i < 5; i++) {
        for (int j = 0; j <= i; j++) {
            printf("* ");
        }
        printf("\n");
    }

    return 0;
}