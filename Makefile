# This is the compiler 
CC = gcc

# Flags for the compiler
CFLAGS = -Wall -Wextra -g

# Rule to build the executable
mysh: main.c
	$(CC) $(CFLAGS) -o mysh main.c

# Cleans the build
clean:
	rm -f mysh