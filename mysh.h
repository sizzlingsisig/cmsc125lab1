#ifndef MYSH_H 
#define MYSH_H

#include <stdbool.h>

#define MAX_CMD_LEN 1024
#define MAX_ARGS 256
#define DEFAULT_FILE_MODE 0644

/**
 * @struct Command
 * @brief Represents a parsed command ready for execution.
 */
typedef struct
{
    char *command;        // Command name
    char *args[MAX_ARGS]; // Arguments (NULL-terminated)
    char *input_file;     // For < redirection (NULL if none)
    char *output_file;    // For > or >> redirection (NULL if none)
    bool append;          // true for >>, false for >
    bool background;      // true if & present
} Command;

/**
 * @enum CommandType
 * @brief Enum for built-in command types.
 */
typedef enum
{
    CMD_EXIT,
    CMD_CD,
    CMD_PWD,
    CMD_EXTERNAL
} CommandType;

// Parser Functions (in parser.c)
void parse_input(char *input, Command *cmd);
void debug_print_command(Command *cmd);

// Executor Functions (in executor.c)
void execute_command(Command *cmd);
void reap_background_processes(void);

#endif // MYSH_H