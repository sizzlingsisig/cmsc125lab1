#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <linux/limits.h>
#include "mysh.h"

/**
 * @brief Determines the type of command.
 */
CommandType get_command_type(const char command[])
{
    if (strcmp(command, "exit") == 0) return CMD_EXIT;
    if (strcmp(command, "cd") == 0)   return CMD_CD;
    if (strcmp(command, "pwd") == 0)  return CMD_PWD;
    return CMD_EXTERNAL;
}

/**
 * @brief Generic helper to open a file and dup2 it.
 */
void redirect_fd(const char *filename, int flags, int target_fd)
{
    if (filename == NULL) return;

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

void handle_redirections(Command *cmd)
{
    redirect_fd(cmd->input_file, O_RDONLY, STDIN_FILENO);

    if (cmd->output_file != NULL)
    {
        int flags = O_WRONLY | O_CREAT;
        if (cmd->append) flags |= O_APPEND;
        else             flags |= O_TRUNC;

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
        if (cmd->args[1] == NULL)
        {
            fprintf(stderr, "mysh: expected argument to \"cd\"\n");
        }
        else
        {
            if (chdir(cmd->args[1]) != 0) perror("mysh cd error");
        }
        return true;

    case CMD_PWD:
        {
            char cwd[PATH_MAX];
            if (getcwd(cwd, sizeof(cwd)) != NULL) printf("%s\n", cwd);
            else perror("getcwd() error");
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

    if (pid < 0)
    {
        perror("fork failed");
    }
    else if (pid == 0)
    {
        handle_redirections(cmd);
        execvp(cmd->command, cmd->args);
        fprintf(stderr, "mysh: command not found: %s\n", cmd->command);
        exit(127);
    }
    else
    {
        if (cmd->background)
        {
            printf("[job %d] %d\n", pid, pid);
        }
        else
        {
            int status;
            waitpid(pid, &status, 0);
        }
    }
}

// TODO: Ensure that pressing Ctrl+C (SIGINT) in the shell doesn't kill the shell itself but correctly interrupts the foreground child process.
/**
 * @brief Makes decision on what type of command to execute.
 */
void execute_command(Command *cmd)
{
    if (cmd->command == NULL) return;
    if (execute_builtin_command(cmd)) return;
    execute_external_command(cmd);
}

void reap_background_processes(void)
{
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        printf("[job %d] finished\n", pid);
    }
}