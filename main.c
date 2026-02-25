#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mysh.h"

int main()
{
    char input[MAX_CMD_LEN];
    Command cmd;

    // This is the REPL of the shell
    while (1)
    {
        reap_background_processes();

        printf("mysh> ");
        fflush(stdout);

        // This checks if enter key is inputted
        if (fgets(input, sizeof(input), stdin) == NULL)
        {
            printf("\n");
            break;
        }

        // Empty input check
        if (strlen(input) <= 1)
            continue;

        parse_input(input, &cmd);
        // debug_print_command(&cmd);
        execute_command(&cmd);
    }

    return 0;
}
