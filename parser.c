#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mysh.h"

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

    while (token != NULL && i < MAX_ARGS - 1)
    {
        if (strcmp(token, ">") == 0)
        { // 1. Check if token is ">" -> Next token is output_file (append=false)
            token = strtok(NULL, " \t");
            cmd->append = false;

            if (token == NULL)
            {
                fprintf(stderr, "mysh: syntax error near unexpected token\n");
                cmd->command = NULL;
                return;
            }
            cmd->output_file = token;
        }
        else if (strcmp(token, ">>") == 0)
        { // 2. Check if token is ">>" -> Next token is output_file (append=true)
            token = strtok(NULL, " \t");
            cmd->append = true;

            if (token == NULL)
            {
                fprintf(stderr, "mysh: syntax error near unexpected token\n");
                return;
            }
            cmd->output_file = token;
        }
        else if (strcmp(token, "<") == 0)
        { // 3. Check if token is "<" -> Next token is input_file
            token = strtok(NULL, " \t");
            cmd->input_file = token;

            if (token == NULL)
            {
                fprintf(stderr, "mysh: syntax error near unexpected token\n");
                return;
            }
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

