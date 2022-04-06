/**
 * @file flush.c
 * @author Seran Shanmugathas (serans@stud.ntnu.no)
 * @author Francin Vincent (francinv@stud.ntnu.no)
 * @brief famous little unix shell
 * @version 0.1
 * @date 2022-03-28
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#include "../../include/core/flush.h"

#define KILOBYTE 1024
#define CONTROL_D 0x04
#define TAB 0x09

/**
 * @brief prompts the user for input by printing the current working directory followed by a colon (:) character.
 * 
 */
void print_current_dir(void)
{
    char cwd[KILOBYTE];
    getcwd(cwd, sizeof(cwd));
    printf("%s: ", cwd);
}

/**
 * @brief Split string by 0x20 (space) or 0x09 (tab).
 * 
 */
void split_string(char *str, char **args) 
{
    int i = 0;
    char *token = strtok(str, " \t");
    while (token != NULL) 
    {
        args[i] = token;
        i++;
        token = strtok(NULL, " \t");
    }
    args[i] = NULL;
}

/**
 * @brief print the exit status of the last executed command, with the command line.
 * This happens when the started (foreground) process terminates.
 * 
 */
void print_exit_status(int status, char **args) 
{
    printf("\nExit status [%s] = %d\n", args[0], WEXITSTATUS(status));
}

/**
 * @brief change the current working directory to the given path.
 * 
 */
void change_dir(char *path) 
{
    if (chdir(path) != 0) 
    {
        perror("Error changing directory");
    }
}

/**
 * @brief check if redirect io
 * 
 */
int check_redirect_io(char **args) 
{
    int i;
    for (i = 0; args[i] != NULL; i++) 
    {
        if (strcmp(args[i], "<") == 0 || strcmp(args[i], ">") == 0) 
        {
            return 1;
        }
    }
    return 0;
}


/**
 * @brief Implement simple I/O redirection for stdin and stdout when 
 * this is indicated on the command line using the < or > characters 
 * followed by a file name.
 * 
 */
void redirect_io(char **args) 
{
    int i;
    for (i = 0; args[i] != NULL; i++) 
    {
        if (strcmp(args[i], "<") == 0) 
        {
            args[i] = args[i + 1];
            freopen(args[i], "r", stdin);
        }
        else if (strcmp(args[i], ">") == 0) 
        {
            args[i] = args[i + 1];
            freopen(args[i], "w", stdout);
        }
    }
}


/**
 * @brief checks for backround process, by checking if 
 * inputs last letter is  "&"
 * 
 */
int check_background(char **args) 
{
    if (strcmp(args[SIZE_OF(args) - 1], "&") == 0) 
    {
        return 1;
    }
    return 0;
}


/**
 * @brief function for finding pipe
 */
int find_pipe(char **args) 
{
    int i;
    for (i = 0; args[i] != NULL; i++) 
    {
        if (strcmp(args[i], "|") == 0) 
        {
            return i;
        }
    }
    return -1;
}


/**
 * @brief Implement the command line parsing, create a new process (using fork(2)) 
 * and execute the entered command with the given parameters (using a variant 
 * of exec(3)).
 *
 */
void execute_command(char **args) 
{
    pid_t pid;
    int status;

    pid = fork();
    if (pid == -1)
    {
        perror("Fork failed");
    }
    else if (pid == 0) 
    {
        if (check_redirect_io(args)) 
        {
            redirect_io(args);
        }
        else if (execvp(args[0], args) == -1) 
        {
            perror("Error");
        }
        exit(EXIT_FAILURE);
    } 
    else if (pid < 0) 
    {
        perror("Error");
    } 
    else 
    {
        if (check_background(args)) 
        {
            int key = length() + 1;
            insertFirst(key, pid, args);
        }
        else 
        {
            waitpid(pid, &status, WUNTRACED);
            if (WIFEXITED(status)) 
            {
                print_exit_status(status, args);
            }
        }
    }
}

/**
 * @brief execute pipeline args
 * 
 */
void execute_pipeline(char **args) 
{
    int pipefd[2];
    pid_t pid;
    int status;

    if (pipe(pipefd) == -1) 
    {
        perror("Error");
        exit(EXIT_FAILURE);
    }

    pid = fork();
    if (pid == 0) 
    {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);
        if (execvp(args[0], args) == -1) 
        {
            perror("Error");
        }
        exit(EXIT_FAILURE);
    } 
    else if (pid < 0) 
    {
        perror("Error");
    } 
    else 
    {
        
        do 
        {
            waitpid(pid, &status, WUNTRACED);
        } 
        while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }
}

/**
 * @brief main method, 
 * check if sig is 0x04 (control-d), if so terminate process
 */
int main(int argc, char *argv[]) {
    char **args;
    char *input;
    int status;
    
    do
    {
        print_current_dir();
        scanf("%[^\n]", input);
        if (input == NULL) 
        {
            printf("\n");
            exit(EXIT_SUCCESS);
            print_exit_status(status, args);
        }
        if (strcmp(input, "") == 0) 
        {
            continue;
        }

        split_string(input, args);
      
        if (strcmp(args[0], "cd") == 0) 
        {
            change_dir(args[1]);
        }
        else if (strcmp(args[0], "exit") == 0) 
        {
            exit(EXIT_SUCCESS);
            print_exit_status(status, args);
        }
        else if (strcmp(args[0], "jobs") == 0) 
        {
            printList();
        }
        else if (find_pipe(args) != -1) 
        {
            execute_pipeline(args);
        } 
        else 
        {
            execute_command(args);
        }
    } while (input[0] != CONTROL_D);
    print_exit_status(status, args);
    return 0;
}