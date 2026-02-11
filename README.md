# CMSC 125 Lab 1: Unix Shell (mysh)

**Course:** CMSC 125 - Operating Systems  
**Instructor:** Rene Jocsing  

**Group Members:**
* Christian Joseph Hernia
* Julo Bretana

## Overview
`mysh` is a custom implementation of a Unix command-line interpreter written in C. This project demonstrates core operating system concepts including process creation (`fork`/`exec`), file descriptor manipulation (`dup2`), and signal handling. It parses user input, manages child processes, and handles file I/O redirection using the POSIX API.

## Task
Build a Unix shell (`mysh`) that supports:
- Interactive command execution with a prompt.
- Built-in commands: `exit`, `cd`, `pwd`.
- External command execution via `fork()` and `exec()`.
- I/O redirection: `>` (truncate), `>>` (append), `<` (input).
- Background execution: `&`.

## Required Features
* **Interactive Command Loop:** A REPL (Read-Eval-Print Loop) that accepts user commands.
* **Built-in Commands:** implementation of `exit`, `cd`, and `pwd` without forking.
* **External Command Execution:** Running system programs (e.g., `ls`, `grep`) using child processes.
* **I/O Redirection:** Redirecting standard input and output using file descriptors.
* **Background Execution:** Running processes asynchronously using the `&` operator and preventing zombie processes.
* **Error Handling:** Graceful handling of syntax errors, missing files, and permission denied errors.

## Implementation Phases
The development of `mysh` follows an incremental strategy to ensure stability at every step:

### Phase 1: The Skeleton & Parser (IMPLEMENTED)
* Implement the main `while` loop and prompt (`mysh> `).
* Parse user input into tokens using `strtok()`.
* Handle the `exit` command to terminate the shell cleanly.

### Phase 2: Basic Execution (Foreground) (IMPLEMENTED)
* Implement the `Command` data structure.
* Add logic for built-in commands: `cd` (using `chdir`) and `pwd` (using `getcwd`).
* Implement `fork()` and `execvp()` for external commands.
* Parent process waits for child completion using `waitpid()`.

### Phase 3: Advanced Parsing (IMPLEMENTED)
* Enhance the parser to detect special tokens (`>`, `>>`, `<`, `&`).
* Populate the `Command` struct with input/output filenames and background flags.
* Clean arguments passed to `execvp` (removing redirection symbols and filenames).

### Phase 4: I/O Redirection (IMPLEMENTED)
* Implement file opening with correct flags (`O_RDONLY`, `O_CREAT`, `O_TRUNC`, `O_APPEND`).
* Use `dup2()` in the child process to redirect `STDIN_FILENO` and `STDOUT_FILENO`.
* Ensure proper closing of unused file descriptors to prevent leaks.

### Phase 5: Background Processing (IMPLEMENTED)
* Implement logic to skip `waitpid()` if the background flag is set.
* Print job ID and PID upon starting a background job.
* Implement a "reaper" function using `waitpid(..., WNOHANG)` to clean up zombie processes at the start of every loop iteration.

### Phase 6: Polish & Testing (IMPLEMENTED)
* Handle edge cases (empty input, multiple spaces).
* Add error handling for failed system calls (e.g., `fork` failing, file not found).
* Verify against the `bash` shell behavior.

## Compilation and Usage
**To compile:**
```bash
make
```
**Running the shell**
```bash
./mysh
```

**Clean**
```bash
make clean
```

## Usage Examples
```bash
mysh> ls -l > output.txt      # Redirect output to file
mysh> grep "code" < main.c    # Read input from file
mysh> sleep 5 &               # Run in background
[job 12345] 12345
mysh> cd /tmp                 # Change directory
```

## Known Limitations
- Piping: The pipe operator (|) is not currently supported.
 - Command History: Arrow key navigation for previous commands is not implemented.
 - String Quoting: Arguments inside quotes (e.g., echo "hello world") are split into separate arguments rather than treated as a single string.

 ## Screenshots
 1. Compilation and Basic Commands
 
 2. Redirection and Background Processing