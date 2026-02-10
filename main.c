#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <fcntl.h>
#include <linux/limits.h>

// TODO: Move constants, structs, enums, and prototypes to separate header file
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

//TODO: handle edge cases where I/O redirection symbols are at the end of the command
//TODO: Phase 6 - Handle backslash escape characters (e.g., "\ ")
//TODO: Move parser to separate parser.c file
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


//TODO: Move execution logic to separate execution.c file
/**
 * @brief Generic helper to open a file and dup2 it to a target file descriptor.
 */
void redirect_fd(const char *filename, int flags, int target_fd)
{
    if (filename == NULL)
        return;

    int fd = open(filename, flags, DEFAULT_FILE_MODE);
    if (fd < 0)
    {
        perror("mysh: redirection error");
        exit(1);
    }

    if (dup2(fd, target_fd) < 0)
    {
        perror("mysh: dup2 failed");
        exit(1);
    }

    close(fd);
}

/**
 * @brief Handles all IO redirection for a command.
 */
void handle_redirections(Command *cmd)
{
    // Handle Input (<)
    redirect_fd(cmd->input_file, O_RDONLY, STDIN_FILENO);

    // Handle Output (> or >>)
    if (cmd->output_file != NULL)
    {
        int flags = O_WRONLY | O_CREAT;

        // Decide between Append (>>) or Truncate (>)
        if (cmd->append)
            flags |= O_APPEND;
        else
            flags |= O_TRUNC;

        redirect_fd(cmd->output_file, flags, STDOUT_FILENO);
    }
}

/**
 * @brief Handles built-in commands (cd, exit, pwd).
 * @return true if the command was a built-in and executed, false otherwise.
 */
bool execute_builtin_command(Command *cmd)
{
    switch (get_command_type(cmd->command))
    {

    case CMD_EXIT:
        printf("Exiting shell...\n");
        exit(0);

    case CMD_CD:
        // Prints error if no argument is provided
        if (cmd->args[1] == NULL)
        {
            fprintf(stderr, "mysh: expected argument to \"cd\"\n");
        }

        else
        {
            int result = chdir(cmd->args[1]);
            if (result != 0)
            {
                perror("mysh cd error");
            }
        }
        return true;

    case CMD_PWD:
        char cwd[PATH_MAX];
        char *result = getcwd(cwd, sizeof(cwd));

        if (result != NULL)
        {
            printf("%s\n", cwd);
        }
        else
        {
            perror("getcwd() error");
        }
        return true;

    case CMD_EXTERNAL:
    default:
        return false;
    }
}

/**
 * @brief Handles external commands using fork, exec, wait.
 */
void execute_external_command(Command *cmd)
{
    pid_t pid = fork();

    // OS fails to create process
    if (pid < 0)
    {
        perror("fork failed");
    }
    // Child process
    else if (pid == 0)
    {
        handle_redirections(cmd);
        execvp(cmd->command, cmd->args);

        // Error handling if exec fails
        fprintf(stderr, "mysh: command not found: %s\n", cmd->command);
        exit(127);
    }
    // Parent process
    else
    {
        // TODO: Handle background jobs (PHASE 5)
        int status;
        waitpid(pid, &status, 0);
    }
}

//TODO: Ensure that pressing Ctrl+C (SIGINT) in the shell doesn't kill the shell itself but correctly interrupts the foreground child process.
/**
 * @brief Makes decision on what type of command to execute.
 */
void execute_command(Command *cmd)
{
    // returns if no command was parsed
    if (cmd->command == NULL)
    {
        return;
    }

    if (execute_builtin_command(cmd))
    {
        return;
    }

    execute_external_command(cmd);
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
