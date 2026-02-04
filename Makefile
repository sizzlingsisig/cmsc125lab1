# Compiler and Flags
CC = gcc
CFLAGS = -Wall -Wextra -g

# Target: all
# Description: Compile the shell
all: mysh

# Rule to build the executable
mysh: main.c
	$(CC) $(CFLAGS) -o mysh main.c

# Target: clean
# Description: Remove binaries and object files
clean:
	rm -f mysh *.o

# helper to prevent conflicts with files named 'all' or 'clean'
.PHONY: all clean