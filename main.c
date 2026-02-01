#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdbool.h>

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
CommandType get_command_type(char *command)
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

    char *token = strtok(input, " \t");
    int i = 0;

    // TODO: (PHASE 3) Improve parser to handle special characters like <, >, >>
    while (token != NULL && i < MAX_ARGS - 1)
    {
        /// 1. Check if token is ">" -> Next token is output_file (append=false)
        // 2. Check if token is ">>" -> Next token is output_file (append=true)
        // 3. Check if token is "<" -> Next token is input_file
        // 4. Check if token is "&" -> Set background=true
        // 5. ELSE -> cmd->args[i++] = token; (Only add non-special tokens to args)

        cmd->args[i++] = token; // need ni ichange
        token = strtok(NULL, " \t");
    }
    cmd->args[i] = NULL;

    if (i > 0)
        cmd->command = cmd->args[0];
}

/**
 * @brief Executes the command found in the Command struct.
 * @param cmd Pointer to the parsed command.
 */
void execute_command(Command *cmd)
{
    // Empty input check
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
            perror("mysh");
        }
        break;

    case CMD_PWD:
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
            //TODO: Handle input redirection (PHASE 4)

            //TODO: Handle output redirection (PHASE 4)
            execvp(cmd->command, cmd->args);
            fprintf(stderr, "mysh: command not found: %s\n", cmd->command);
            exit(127);
        }
        else
        {
            //TODO: Handle background jobs (PHASE 5)
            int status;
            waitpid(pid, &status, 0);
        }
        break;
    }
}

int main()
{
    char input[MAX_CMD_LEN];
    Command cmd;

    // This is the REPL of the shell
    while (1)
    {
        //TODO: Check for Zombie processes (PHASE 5)
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
        execute_command(&cmd);
    }

    return 0;
}