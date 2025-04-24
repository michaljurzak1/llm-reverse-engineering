#include <stdio.h>
#include <string.h>

#define MAX_NAME 50

int factorial(int n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}

void print_pattern(int rows) {
    printf("\nHere's a small pattern for you:\n");
    for (int i = 0; i < rows && i < 5; i++) {
        for (int j = 0; j <= i; j++) {
            printf("* ");
        }
        printf("\n");
    }
}

void get_user_name(char *name) {
    printf("Please enter your name: ");
    fgets(name, MAX_NAME, stdin);
    name[strcspn(name, "\n")] = 0; // Remove trailing newline
}

int get_number(const char *name) {
    int number;
    printf("Hi %s! Enter a number to calculate its factorial: ", name);
    scanf("%d", &number);
    return number;
}

void calculate_factorial(int number) {
    if (number < 0) {
        printf("Sorry, factorial is not defined for negative numbers.\n");
    } else {
        printf("The factorial of %d is %d\n", number, factorial(number));
    }
}

int main() {
    char name[MAX_NAME];
    int number;

    printf("Welcome to the Demo Program!\n");
    
    get_user_name(name);
    number = get_number(name);
    calculate_factorial(number);
    print_pattern(number);

    return 0;
}