
**1. Problem Analysis**
The objective is to develop a custom command-line interpreter that interfaces between the user and the OS kernel. The shell must manage process creation, command execution, and resource handling while maintaining stability.

*Key Technical Challenges Identified*
    - Parsing Logic: Standard string splitting is insufficient. The parser must distinguish between arguments (flags) and control operators (>, <, &) to prevent errors during execution.

    - Process Management: The shell must use fork() to isolate command execution so that errors in child processes do not crash the main shell.

    - Resource Handling: Future implementation of redirection requires precise management of file descriptors using dup2() to avoid resource leaks.

    - Concurrency: Implementing background jobs (&) requires a non-blocking "reaper" mechanism to prevent zombie processes from consuming system resources.

**2. Solution Architecture**
We have adopted a modular architecture that separates data storage, parsing, and execution.

*A. Data Structure (struct Command)*
    To maintain state between the parser and executor, we defined a centralized structure:

    C
    typedef struct {
        char *command;        // Primary command binary
        char *args[256];      // Arguments vector for execvp
        char *input_file;     // Path for Input Redirection (<)
        char *output_file;    // Path for Output Redirection (>)
        bool append;          // Flag for Append Mode (>>)
        bool background;      // Flag for Asynchronous Execution (&)
    } Command;

*B. The Parser (Look-Ahead Tokenizer)*
    - Mechanism: Iterates through tokens using strtok.

    - Logic: Checks for special characters (>, >>, <, &). If found, it captures the subsequent token as a filename or sets the corresponding flag, removing these tokens from the argument list passed to execvp.

*C. The Executor (Router Pattern)*
    - Built-ins: Commands like cd, pwd, and exit are executed directly in the parent process.

    - External Commands:
        > Foreground: Uses fork() to create a child, execvp() to run the binary, and waitpid() to block until completion.
        > Planned Features: Logic for dup2 (redirection) and WNOHANG (background jobs) is currently being architected for the next phase.

**3. Estimated Implementation Timeline**
*Phase 1*
    Description: Skeleton & Basic Parser (REPL, strtok, exit)
    Status: **Completed - Week 1**

*Phase 2*
    Description: Basic Execution (Foreground commands, Built-ins)
    Status: **Completed - Week 1**

*Phase 3*
    Description: Advanced Parsing (Handling >,<,& tokens)
    Status: **Completed - Week 1**

*Phase 4*
    Description: I/O Redirection (open, dup2 logic)
    Status: **Completed - Week 2**

*Phase 5*
    Description: Background Processing (Zombie reaper, & logic)
    Status: **Completed - Week 2**

*Phase 6*
Description: Polish, Edge Case Testing & Documentation
    Status: **Completed - Week 2**
