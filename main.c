#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <fcntl.h>

// TODO: Maybe move constants to a header file
#define MAX_CMD_LEN 1024
#define MAX_ARGS 256

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

/**
 * @brief Determines the type of command.
 * @param command Command name string.
 * @return CommandType enum value.
 */
CommandType get_command_type(const char command[])
{
    if (strcmp(command, "exit") == 0)
        return CMD_EXIT;
    if (strcmp(command, "cd") == 0)
        return CMD_CD;
    if (strcmp(command, "pwd") == 0)
        return CMD_PWD;
    return CMD_EXTERNAL;
}

/**
 * @brief Parses raw input into a Command struct.
 * @param input Raw input string from fgets.
 * @param cmd   Pointer to the struct to fill.
 */
void parse_input(char *input, Command *cmd)
{
    memset(cmd, 0, sizeof(Command));

    input[strcspn(input, "\n")] = 0;
    // FIXME: \C \K handling
    char *token = strtok(input, " \t");
    int i = 0;

    while (token != NULL && i < MAX_ARGS - 1)
    {
        if (strcmp(token, ">") == 0)
        { // 1. Check if token is ">" -> Next token is output_file (append=false)
            token = strtok(NULL, " \t");
            cmd->output_file = token;
            cmd->append = false;
        }
        else if (strcmp(token, ">>") == 0)
        { // 2. Check if token is ">>" -> Next token is output_file (append=true)
            token = strtok(NULL, " \t");
            cmd->output_file = token;
            cmd->append = true;
        }
        else if (strcmp(token, "<") == 0)
        { // 3. Check if token is "<" -> Next token is input_file
            token = strtok(NULL, " \t");
            cmd->input_file = token;
        }
        else if (strcmp(token, "&") == 0)
        { // 4. Check if token is "&" -> Set background=true
            cmd->background = true;
        }
        else
        { // 5. ELSE -> cmd->args[i++] = token; (Only add non-special tokens to args)
            cmd->args[i++] = token;
        }

        token = strtok(NULL, " \t");
    }
    cmd->args[i] = NULL;

    if (i > 0)
    {
        cmd->command = cmd->args[0];
    }
}

/**
 * @brief Handles input and output redirection based on the Command struct.
 * @param cmd Pointer to the parsed command.
 */
void handle_redirection(Command *cmd)
{
    if (cmd->input_file != NULL)
    {
        int input_fd = open(cmd->input_file, O_RDONLY);
        if (input_fd < 0)
        {
            perror("mysh: input redirection error");
            exit(1);
        }

        if (dup2(input_fd, STDIN_FILENO) < 0)
        {
            perror("mysh: dup2 input failed");
            exit(1);
        }

        close(input_fd);
    }

    if (cmd->output_file != NULL)
    {
        int flags;
        // FIXME: replace magic number
        mode_t mode = 0644;

        /**
         * Case: >> (append mode)
         * O_WRONLY: Open for writing only.
         * O_CREAT: Create the file if it does not exist.
         * O_APPEND: Append to the end of the file if it exists (for >>).
         */
        if (cmd->append)
        {
            flags = O_WRONLY | O_CREAT | O_APPEND;
        }
        /**
         * Case: > (truncate mode)
         * O_WRONLY: Open for writing only.
         * O_CREAT: Create the file if it does not exist.
         * O_TRUNC: Truncate the file to zero length if it already exists (for >).
         */
        else
        {
            flags = O_WRONLY | O_CREAT | O_TRUNC;
        }

        int output_fd = open(cmd->output_file, flags, mode);
        if (output_fd < 0)
        {
            perror("mysh: output redirection error");
            exit(1);
        }

        // Redirect Standard Output (1) to this file
        if (dup2(output_fd, STDOUT_FILENO) < 0)
        {
            perror("mysh: dup2 output failed");
            exit(1);
        }

        close(output_fd);
    }
}
/**
 * @brief Executes the command found in the Command struct.
 * @param cmd Pointer to the parsed command.
 */
void execute_command(Command *cmd)
{
    if (cmd->command == NULL)
        return;

    switch (get_command_type(cmd->command))
    {
    case CMD_EXIT:
        printf("Exiting shell...\n");
        exit(0);
        break;
    case CMD_CD:
        if (cmd->args[1] == NULL)
        {
            fprintf(stderr, "mysh: expected argument to \"cd\"\n");
        }
        else if (chdir(cmd->args[1]) != 0)
        {
            // FIXME: remove perror since this is success
            perror("mysh");
        }
        break;

    case CMD_PWD:
        // FIXME: Use the standard constant
        char cwd[MAX_CMD_LEN];
        if (getcwd(cwd, sizeof(cwd)) != NULL)
        {
            printf("%s\n", cwd);
        }
        else
        {
            perror("getcwd() error");
        }
        break;

    case CMD_EXTERNAL:
        pid_t pid = fork();
        if (pid < 0)
        {
            perror("fork failed");
        }
        else if (pid == 0)
        {
            handle_redirection(cmd);
            execvp(cmd->command, cmd->args);
            fprintf(stderr, "mysh: command not found: %s\n", cmd->command);
            exit(127);
        }
        else
        {
            // TODO: Handle background jobs (PHASE 5)
            int status;
            waitpid(pid, &status, 0);
        }
        break;
    }
}

void debug_print_command(Command *cmd)
{
    printf("\n--- DEBUG: PARSER STATUS ---\n");
    printf("Command:      [%s]\n", cmd->command ? cmd->command : "NULL");
    printf("Args:         ");
    for (int i = 1; cmd->args[i] != NULL; i++)
    {
        printf("[%s] ", cmd->args[i]);
    }
    printf("\n");

    printf("Input File:   [%s]\n", cmd->input_file ? cmd->input_file : "None");
    printf("Output File:  [%s]\n", cmd->output_file ? cmd->output_file : "None");
    printf("Append Mode:  [%s]\n", cmd->append ? "YES" : "NO");
    printf("Background:   [%s]\n", cmd->background ? "YES" : "NO");
    printf("----------------------------\n\n");
}

int main()
{
    char input[MAX_CMD_LEN];
    Command cmd;

    // This is the REPL of the shell
    while (1)
    {
        // TODO: Check for Zombie processes (PHASE 5)
        printf("mysh> ");
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL)
        {
            printf("\n");
            break;
        }

        // Empty input check
        if (strlen(input) <= 1)
            continue;

        parse_input(input, &cmd);
        debug_print_command(&cmd);
        execute_command(&cmd);
    }

    return 0;
}
