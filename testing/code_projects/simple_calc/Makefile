CC = gcc
CFLAGS = -Wall -Wextra

all: simple_calc

simple_calc: main.c factorial.c
	$(CC) $(CFLAGS) -o simple_calc main.c factorial.c

clean:
	rm -f simple_calc

.PHONY: all clean 