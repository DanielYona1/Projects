#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pwd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
void printPrompt()
{

    struct passwd *pwd;
    pwd = getpwuid(getuid());
    long size = pathconf(".", _PC_PATH_MAX);
    char* cwd = (char*)malloc(150);
    if (cwd != NULL)
    {
        printf("%s@%s>", pwd->pw_name, getcwd(cwd, (size_t)size));
    }
}

int main(int argc, char *argv[])
{
    int commands_counter = 0; // How many commands.
    int length_counter = 0;   // Length in bytes of all commands.
    int done = 1;             // As long as value is '1' keep taking commands from the user and execute, when done will change to '0'
    int pipe_counter = 0;     // counting commands with pipe
    int redirection_counter = 0; // counting commands with redirection
    while (done == 1)
    {
        int i;
        int k;
        int ifpipe = 0;        // if the commands contains "|" it will be '1'
        int ifgadol = 0;       // if the commands contains ">" pipe it will be '1'
        int ifkatan = 0;       // // if the commands contains "<" pipe it will be '1'
        int ifgadolgadol = 0;  // if the commands contains ">>" pipe it will be '1'
        int if2gadol = 0;      // if the commands contains "2>" pipe it will be '1'
        int spaces_counter = 0;
        commands_counter++;
        printPrompt();                                  // A function that will print the prompt line of the shell: " user@path> "
        char *cmd = (char *)malloc(sizeof(char) * 510); // Allocate memory to the cmd, maximum size of 510 bytes.
        if (cmd == NULL)
        {
            printf("array not aloccated");
            exit(1);
        }
        fgets(cmd, 510, stdin); // Scan the command from the user.
        if (strcmp(cmd, "\n") == 0)
        {
            free(cmd);
            continue;
        }

        length_counter += strlen(cmd);
        cmd = (char *)realloc(cmd, strlen(cmd) * sizeof(char)); //Reallocate the memory so it will not use all the 510 allocated before.
        int log = open(argv[1] , O_WRONLY | O_CREAT | O_APPEND , S_IRWXU);
        write(log , cmd , sizeof(char)*strlen(cmd)); // writing the commands in the file in argv[1]
        for (i = 0; i < strlen(cmd); i++) // if the command is without spaces it fix the command to be with spaces
        {
            char * newcmd;

            if(cmd[i] == '|' && (cmd[i-1] != ' ' && cmd[i+1] != ' ')) // if there are two spaces before and after the pipe
            {
                pipe_counter++;
                char* newcmd = (char*)malloc(strlen(cmd) + 2);
                for(k = 0 ; k < i ; k++ )
                {
                    newcmd[k] = cmd[k];
                }
                newcmd[k] = ' ';
                k++;
                if (cmd[i] == '|')
                    newcmd[k] = '|';
                k++;
                newcmd[k] = ' ';
                k++; 
                for( ; k < strlen(cmd) + 2 ; k++ , i++)
                {
                    newcmd[k] = cmd[i+1];
                }
                cmd = (char *)realloc(newcmd, strlen(newcmd));
            }
            else if (cmd[i] == '|'  && cmd[i - 1] != ' ') // if the commands has no space only before the pipe
            {
                char *newcmd = (char *)malloc(sizeof(char) * (strlen(cmd) + 1));
                for (k = 0; k < i; k++)
                {
                    newcmd[k] = cmd[k];
                }
                newcmd[i] = ' ';
                k++;
                for (; k < strlen(cmd) + 1 ; k++, i++)
                {
                    newcmd[k] = cmd[i];
                }
                cmd = (char *)realloc(newcmd, strlen(newcmd));
            }
            else if (cmd[i] == '|' && cmd[i + 1] != ' ') // if the command has no space only after the pipe
            {
                char *newcmd = (char *)malloc(sizeof(char) * (strlen(cmd) + 1));
                for (k = 0; k < i + 1; k++)
                {
                    newcmd[k] = cmd[k];
                }
                newcmd[k] = ' ';
                k++;

                for (; k < strlen(cmd) + 1; k++, i++)
                {
                    newcmd[k] = cmd[i + 1];
                }
                cmd = (char *)realloc(newcmd, strlen(newcmd));
            }
            

            
        }
if (cmd == NULL)
{
    printf("array not aloccated");
    exit(1);
}
cmd[strlen(cmd) - 1] = '\0';          //Manually put NULL in the last index.
for (int i = 0; i < strlen(cmd); i++) // A loop to count the spaces to know how many arguments there are.
{
    if (cmd[i] == ' ')
        spaces_counter++;
}
spaces_counter += 2;
char **commands = (char **)malloc(sizeof(char *) * spaces_counter); //Allocate the char** array in the size of the arguments given by user.
if (commands == NULL)
{
    printf("array not aloccated");
    exit(1);
}
char *token = strtok(cmd, " "); //Cut the first argument from the whole command.
int index = 0;
int token_size;
while (token != NULL)
{

    token_size = strlen(token);
    commands[index] = (char *)malloc(sizeof(char) * token_size); //Allocate the inner array for each speciefic argument in the correct size.
    strcpy(commands[index], token);
    index++;
    token = strtok(NULL, " "); //Cut the next word from the command.
}


if (strcmp(commands[0], "cd") == 0)
{
    printf("Not supported (yet) \n");
    for (int j = 0; j < spaces_counter; j++)
    {
        free(commands[j]);
    }
    free(cmd);
    free(token);
    free(commands);
    continue;
}

if (strcmp(commands[0], "done") == 0)
{
    done = 0;
    double avg = (double)length_counter / (double)commands_counter;
    printf("Number of commands: %d \n", commands_counter);
    printf("Total length: %d \n", length_counter);
    printf("Average length per command: %f \n", avg);
    printf("Number of commands that include pipe: %d \n" , pipe_counter);
    printf("Number of commands that include redirection: %d \n"  , redirection_counter);
    printf("See you next time. \n");
    for (int j = 0; j < spaces_counter; j++)
    {
        free(commands[j]);
    }
    free(cmd);
    free(token);
    free(commands);
    exit(0);
}
else if (strcmp(commands[0], "done") != 0 && strcmp(commands[0], "cd") != 0)
{
    commands[index] = NULL;

    for (i = 0; i < index; i++)
    {
        // cheking if there is pipe or redirection if true copy the command untill the pipe/redirection to command1 and after the pipe/redirection to command2
        if (strcmp(commands[i], "|") == 0 || strcmp(commands[i] , ">>") == 0 || strcmp(commands[i] , "2>") == 0 || strcmp(commands[i] , "<") == 0 || strcmp(commands[i] , ">") == 0)
        {
            char **commands1;
            char **commands2;
            if(strcmp(commands[i] , ">>") == 0 || strcmp(commands[i] , "2>") == 0 )
            {
                if(strcmp(commands[i] , ">>") == 0)
                {
                    redirection_counter++;
                    ifgadolgadol = 1;
                }

                else if(strcmp(commands[i] , "2>") == 0)
                {
                    redirection_counter++;
                    if2gadol = 1;
                }
                commands1 = (char **)malloc(sizeof(char *) * i);
                commands1[i] = NULL;
            
                for (int j = 0; j < i; j++)
                {
                    commands1[j] = (char *)malloc(sizeof(char) * strlen(commands[j]));
                    strcpy(commands1[j], commands[j]);
                }
                commands2 = (char **)malloc(sizeof(char *) * (index - i));
                commands2[index - i] = NULL;
                for (k = 0; k < index - i - 1; k++)
                {
                    commands2[k] = (char *)malloc(sizeof(char) * strlen(commands[i + k + 1]));
                    strcpy(commands2[k], commands[i + k + 1]);
                }
            }

            else 
            {
                if(strcmp(commands[i] , ">") == 0)
                {
                    redirection_counter++;
                    ifgadol = 1;
                }
                else if(strcmp(commands[i] , "|") == 0)
                {
                    pipe_counter++;
                    ifpipe = 1;
                }
                else if(strcmp(commands[i] , "<") == 0)
                {
                    redirection_counter++;
                    ifkatan = 1;
                }
                commands1 = (char **)malloc(sizeof(char *) * i);
                commands1[i] = NULL;
                for (int j = 0; j < i; j++)
                {
                    commands1[j] = (char *)malloc(sizeof(char) * strlen(commands[j]));
                    strcpy(commands1[j], commands[j]);
                }
                commands2 = (char **)malloc(sizeof(char *) * (index - i - 1));
                for (k = 0; k < index - i - 1; k++)
                {
                    commands2[k] = (char *)malloc(sizeof(char) * strlen(commands[i + k + 1]));
                    strcpy(commands2[k], commands[i + k + 1]);
                }
            }

            // checking if the commands is pipe and redirection together and update the counter
            if(k>1 && (strcmp(commands2[k-2] , ">") == 0 || strcmp(commands2[k-2] , ">>") == 0 || strcmp(commands2[k-2] , "2>") == 0) )
                redirection_counter++;
            int fd[2];
            if (pipe(fd) == -1)
            {
                perror("cant open pipe");
                exit(EXIT_FAILURE);
            }
            pid_t pid1, pid2;
            pid1 = fork();
            if (pid1 == 0)
            {
                int txt;
                if (ifpipe == 1)
                {
                    close(fd[0]);    // close read
                    dup2(fd[1], STDOUT_FILENO); // write to pipe
                    execvp(commands1[0], commands1);
                    close(fd[1]);
                    exit(0);
                }
                if (ifgadol == 1)
                { 
                    
                    close(fd[0]); // close read
                    txt = open(commands2[0] , O_WRONLY | O_CREAT | O_TRUNC , S_IRWXU);
                    dup2(txt , STDOUT_FILENO); // write to txt
                    execvp(commands1[0], commands1);
                }
                if(ifgadolgadol == 1)
                {
                    
                    txt = open(commands2[0] , O_WRONLY | O_CREAT | O_APPEND , S_IRWXU);
                    dup2(txt , STDOUT_FILENO); //write to txt
                    execvp(commands1[0], commands1);
                    break;
                }
                        
                if(if2gadol == 1)
                {
                       
                    txt = open(commands2[0] , O_WRONLY | O_CREAT | O_TRUNC , S_IRWXU);
                    dup2(txt , STDERR_FILENO); // write to text
                    execvp(commands1[0], commands1);
                    break;
                }
                if(ifkatan == 1)
                {
                    
                    close(fd[1]);
                    int txt = open(commands2[0] , O_RDONLY | O_CREAT , S_IRWXU);
                    dup2(txt , STDIN_FILENO); // get input from txt
                    execvp(commands1[0], commands1);
                    close(fd[0]);
                    exit(0);
                }

                close(fd[1]);
                exit(0);
            }
            
            if (ifpipe == 1)
            {
                pid2 = fork();
            }
            if (pid2 == 0)
            {   
                if(k > 1 && strcmp(commands2[k-2] , ">") == 0) // checking pipe and redirection together
                {
                    
                    close(fd[1]);
                    int txt = open(commands2[k-1] , O_WRONLY | O_CREAT | O_TRUNC , S_IRWXU);
                    commands2[k-2] = NULL;
                    dup2(fd[0], STDIN_FILENO); // read from pipe
                    dup2(txt , STDOUT_FILENO);  // write to txt
                    execvp(commands2[0], commands2);
                    close(fd[0]);
                    exit(0);
                }

                else if (k > 1 && strcmp(commands2[k-2] , ">>") == 0) // checking pipe and redirection together
                {
                    
                    close(fd[1]);
                    int txt = open(commands2[k-1] , O_WRONLY | O_CREAT | O_APPEND , S_IRWXU);
                    commands2[k-2] = NULL;
                    dup2(fd[0], STDIN_FILENO); // read from pipe
                    dup2(txt , STDOUT_FILENO); // write to txt
                    execvp(commands2[0], commands2);
                    close(fd[0]);
                    exit(0);
                }

                else if (k > 1 && strcmp(commands2[k-2] , "2>") == 0) // checking pipe and redirection together
                {
                    
                    close(fd[1]);
                    int txt = open(commands2[k-1] , O_WRONLY | O_CREAT | O_APPEND , S_IRWXU);
                    commands2[k-2] = NULL;
                    dup2(fd[0], STDIN_FILENO); // read from pipe
                    dup2(txt , STDERR_FILENO); // write to txt
                    execvp(commands2[0], commands2);
                    close(fd[0]);
                    exit(0);
                }
                else
                { 
                    close(fd[1]);
                    dup2(fd[0], STDIN_FILENO);
                    execvp(commands2[0], commands2);
                    close(fd[0]);
                    exit(0); 
                }
                
            }

            else if (getpid() != 0)
            {
                close(fd[0]);
                close(fd[1]);
                wait(NULL);
                wait(NULL);
                break;
            }
        }
    }
}
    
// if there is no pipe or redirection at all
if (ifpipe == 0 && ifgadol == 0 && ifkatan == 0 && ifgadolgadol == 0 & if2gadol == 0)
{
    pid_t pid;
    pid = fork();
    if (pid == 0)
    {
        execvp(commands[0], commands);
        exit(0);
    }
    else if (getpid() != 0) // Force the father to wait until son is done.
    {
        wait(NULL);
        for (int j = 0; j < spaces_counter; j++)
        {
            free(commands[j]);
        }
        free(cmd);
        free(token);
        free(commands);
    }
}
}
}
