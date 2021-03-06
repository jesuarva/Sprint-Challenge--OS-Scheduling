#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>

#define PROMPT "lambda-shell$ "

#define MAX_TOKENS 100
#define COMMANDLINE_BUFSIZE 1024
#define DEBUG 1 // Set to 1 to turn on some debugging output, or 0 to turn off

/**
 * Parse the command line.
 *
 * YOU DON'T NEED TO MODIFY THIS!
 * (But you should study it to see how it works)
 *
 * Takes a string like "ls -la .." and breaks it down into an array of pointers
 * to strings like this:
 *
 *   args[0] ---> "ls"
 *   args[1] ---> "-la"
 *   args[2] ---> ".."
 *   args[3] ---> NULL (NULL is a pointer to address 0)
 *
 * @param str {char *} Pointer to the complete command line string.
 * @param args {char **} Pointer to an array of strings. This will hold the result.
 * @param args_count {int *} Pointer to an int that will hold the final args count.
 *
 * @returns A copy of args for convenience.
 */
char **parse_commandline(char *str, char **args, int *args_count)
{
    char *token;

    *args_count = 0;

    token = strtok(str, " \t\n\r");

    while (token != NULL && *args_count < MAX_TOKENS - 1)
    {
        args[(*args_count)++] = token;

        token = strtok(NULL, " \t\n\r");
    }

    args[*args_count] = NULL;

    return args;
}

/**
 * Main
 */
int main(void)
{
    // Holds the command line the user types in
    char commandline[COMMANDLINE_BUFSIZE];

    // Holds the parsed version of the command line
    char *args[MAX_TOKENS];

    // How many command line args the user typed
    int args_count;

    // Shell loops forever (until we tell it to exit)
    while (1)
    {
        // Print a prompt
        printf("%s", PROMPT);
        fflush(stdout); // Force the line above to print

        // Read input from keyboard
        fgets(commandline, sizeof commandline, stdin);

        // Exit the shell on End-Of-File (CRTL-D)
        if (feof(stdin))
        {
            break;
        }

        // Parse input into individual arguments
        parse_commandline(commandline, args, &args_count);

        if (args_count == 0)
        {
            // If the user entered no commands, do nothing
            continue;
        }

        // Exit the shell if args[0] is the built-in "exit" command
        if (strcmp(args[0], "exit") == 0)
        {
            break;
        }

        if (strcmp(args[0], "cd") == 0)
        {
            if (args_count == 1)
            {
                perror("Missing path, please pass a path.\n");
                continue;
            }
            else
            {
                chdir(args[1]);
                continue;
            }
        }

#if DEBUG

        // Some debugging output

        // Print out the parsed command line in args[]
        for (int i = 0; args[i] != NULL; i++)
        {
            printf("%d: '%s'\n", i, args[i]);
        }

#endif

        /* Add your code for implementing the shell's logic here */
        int forked_p = fork();

        // Stretch Goal 1:
        int last_argument = strcmp(args[args_count - 1], "&");
        if (last_argument == 0)
        {
            args[args_count - 1] = NULL;
        }

        // Stretch Goal 2:
        int i, file_redirection = 0;
        char file_name[124];
        for (i = 0; args[i] != NULL; i++)
        {
            if (strcmp(args[i], ">") == 0)
            {
                file_redirection = 1;
                args[i] = NULL;
                strcpy(file_name, args[i + 1]);
            }
            if (file_redirection == 1)
            {
                args[i] = NULL;
            }
        }

        if (forked_p < 0)
        {
            printf("PARENT FORK failed.\n");
        }
        else if (forked_p == 0)
        {
            printf("=== %d CHILD_START fork1 ===\n", (int)getpid());

            // Strech Goal 2:
            if (file_redirection == 1)
            {
                printf("FILE REDIRECT == 1\n\n");
                int file_to_output = open(file_name, O_WRONLY | O_CREAT, 0644);
                if (file_to_output < 0)
                {
                    perror("Error writing to lutput_file ( command > output_file ).");
                    exit(1);
                }

                dup2(file_to_output, 1); // close 'fd' number 1. and points this 'number (1)' to the 'file_to_output'.
            }

            execvp(args[0], args);
            printf("=== CHILD_ERROR caling 'execvp' ===\n");
            exit(1);
        }
        else
        {
            printf("=== %d PARENT_START ===\n", (int)getpid());
            if (last_argument != 0) // if the program is not needed to run in the background call 'waitpid'
            {
                printf("=== PARENT_WAITING FOR CHILD ===\n");
                waitpid(forked_p, NULL, 0);
            }
            printf("=== PARENT_END ===\n");
        }
    }

    return 0;
}