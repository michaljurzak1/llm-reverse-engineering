# simple_c_program

## Project Description  
simple_c_program is a small console application originally provided as a binary. The program welcomes the user, prompts for a name and a non-negative integer, computes the factorial of the integer, and then displays a simple left-aligned triangle pattern of asterisks of height equal to the entered number.

## Inferred Functionality  
- Displays a welcome message.  
- Reads the user’s name (up to 49 characters) and strips any trailing newline.  
- Prompts for an integer input and verifies it is valid.  
- If the integer is non-negative, computes its factorial using a recursive routine and prints the result.  
- If the integer is negative or invalid, displays an appropriate error message.  
- Finally, prints a triangle of “* ” symbols with as many rows as the entered number.  

## Build Instructions  
1. Ensure you have a C compiler (GCC) installed.  
2. From the directory containing `simple_c_program.c`, run:  
   gcc -std=c11 -Wall -Wextra -O2 -o simple_c_program simple_c_program.c  
3. An executable named `simple_c_program` will be generated.

## Usage  
1. Launch the program:  
   ./simple_c_program  
2. Follow on-screen prompts:  
   - Enter your name.  
   - Enter a non-negative integer.  
3. View the computed factorial and the generated asterisk pattern.

## Security Considerations  
- Name input is safely bounded by using `fgets` on a 50-byte buffer and stripping the newline.  
- Integer input is read via `scanf("%d")`; invalid inputs are detected but additional sanitization (e.g., rejecting extremely large values) is not implemented.  
- Recursive factorial on large integers may lead to stack overflow or long execution times.  
- Factorial results can overflow a 32-bit `int` or 64-bit `long` for moderately large inputs; no overflow checks are present.  

Please review and adjust input constraints if deploying in a security-sensitive context.