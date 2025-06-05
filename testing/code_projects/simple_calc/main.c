#include <stdio.h>
#include <stdlib.h>

extern int factorial(int n);

int main() {
    int num;
    printf("Enter a number: ");
    scanf("%d", &num);
    printf("Factorial of %d is %d\n", num, factorial(num));
    return 0;
} 