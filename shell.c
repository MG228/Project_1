#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

#define MAX_COMMAND_LINE_LEN 1024
#define MAX_COMMAND_LINE_ARGS 128

char prompt[] = "> ";
char delimiters[] = " \t\r\n";

// Function to handle built-in commands
void handle_builtin_command(char *args[]) {
    if (strcmp(args[0], "cd") == 0) {
        if (args[1] == NULL) {
            fprintf(stderr, "cd: missing argument\n");
        } else {
            if (chdir(args[1]) != 0) {
                perror("cd");
            }
        }
    } else if (strcmp(args[0], "pwd") == 0) {
        char cwd[MAX_COMMAND_LINE_LEN];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s\n", cwd);
        } else {
            perror("getcwd");
        }
    } else if (strcmp(args[0], "echo") == 0) {
        for (int i = 1; args[i] != NULL; i++) {
            printf("%s ", args[i]);
        }
        printf("\n");
    } else if (strcmp(args[0], "exit") == 0) {
        exit(0);
    } else {
        // Handle other built-in commands or show an error message
        fprintf(stderr, "Unknown command: %s\n", args[0]);
    }
}

int main() {
    char command_line[MAX_COMMAND_LINE_LEN];
    char *arguments[MAX_COMMAND_LINE_ARGS];

    while (true) {
        // Print the shell prompt.
        char cwd[MAX_COMMAND_LINE_LEN];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s%s", cwd, prompt);
        } else {
            perror("getcwd");
        }
        fflush(stdout);

        // Read input from stdin and store it in command_line.
        if (fgets(command_line, MAX_COMMAND_LINE_LEN, stdin) == NULL) {
            break; // Exit the shell on EOF (e.g., Ctrl+D)
        }

        // Tokenize cli
        int numTokens = 0;
        char *token = strtok(command_line, delimiters);

        while (token != NULL) {
            arguments[numTokens] = token;
            token = strtok(NULL, delimiters);
            numTokens++;
        }
        arguments[numTokens] = NULL;

        if (numTokens == 0) {
            continue; 
        }

        // Check if the command should run in the background
        int background = 0;
        if (strcmp(arguments[numTokens - 1], "&") == 0) {
            background = 1;
            arguments[numTokens - 1] = NULL;
        }

        // Handle built-in commands
        handle_builtin_command(arguments);

        // non-built-in commands
        if (arguments[0] != NULL) {
            pid_t pid = fork();

            if (pid == -1) {
                perror("fork");
                exit(1);
            } else if (pid == 0) {
                // Child process
                execvp(arguments[0], arguments);
                perror("execvp");
                exit(1);
            } else {
                // Parent process
                if (!background) {
                    int status;
                    if (waitpid(pid, &status, 0) == -1) {
                        perror("wait");
                        exit(1);
                    }
                }
            }
        }
    }

    return 0;
}
