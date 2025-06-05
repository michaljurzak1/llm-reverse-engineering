#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#define MAX_INPUT 100

// Function to validate numeric input
int is_valid_number(const char* str) {
    if (!str || !*str) return 0;
    
    // Remove trailing newline if present
    char* newline = strchr(str, '\n');
    if (newline) *newline = '\0';
    
    // Check for optional sign
    if (*str == '+' || *str == '-') str++;
    
    // Must have at least one digit
    if (!isdigit(*str)) return 0;
    
    // Check remaining characters
    while (*str) {
        if (!isdigit(*str) && *str != '.') return 0;
        str++;
    }
    return 1;
}

// Function to perform calculation
double calculate(double a, double b, char op) {
    switch (op) {
        case '+':
            return a + b;
        case '-':
            return a - b;
        case '*':
            return a * b;
        case '/':
            if (b == 0) {
                printf("Error: Division by zero\n");
                exit(1);
            }
            return a / b;
        case '%':
            if (b == 0) {
                printf("Error: Modulo by zero\n");
                exit(1);
            }
            return (int)a % (int)b;
        case '^':
            {
                double result = 1;
                for (int i = 0; i < (int)b; i++) {
                    result *= a;
                }
                return result;
            }
        default:
            printf("Error: Invalid operator\n");
            exit(1);
    }
}

int main() {
    char input[MAX_INPUT];
    double num1, num2;
    char op;
    
    printf("Simple Calculator\n");
    printf("Operations: +, -, *, /, %%, ^\n");
    printf("Enter 'q' to quit\n\n");
    
    while (1) {
        printf("Enter first number: ");
        if (!fgets(input, MAX_INPUT, stdin)) break;
        if (input[0] == 'q' || input[0] == 'Q') break;
        
        if (!is_valid_number(input)) {
            printf("Invalid number format\n");
            continue;
        }
        num1 = atof(input);
        
        printf("Enter operator: ");
        if (!fgets(input, MAX_INPUT, stdin)) break;
        op = input[0];
        
        printf("Enter second number: ");
        if (!fgets(input, MAX_INPUT, stdin)) break;
        if (!is_valid_number(input)) {
            printf("Invalid number format\n");
            continue;
        }
        num2 = atof(input);
        
        double result = calculate(num1, num2, op);
        printf("Result: %.2f\n\n", result);
    }
    
    return 0;
} 