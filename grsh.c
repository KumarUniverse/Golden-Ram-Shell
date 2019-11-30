/*
 * Filename: grsh.c
 * Author: Akash Kumar
 * Class: CSC 331
 * Date: 11/27/2019
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

// Builtin functions:
void exit_grsh(char **args);
void cd(char **args);
void set_path(char **args);

const char *builtin_cmds[] = {"exit","cd","path"}; // array of strings.
const void (*builtin_cmd_funcs[]) (char **) = {&exit_grsh, &cd, &set_path}; // array of pointers to functions.
char *path = "/bin"; // Default path to executable commands.
const char error_message[30] = "An error has occurred\n";

// A fn that is used to exit the shell.
void exit_grsh(char **args)
{
  exit(0);
}

// A fn that is used to change directories.
void cd(char **args)
{
  if (args[1] == NULL || chdir(args[1]) != 0)
  {
    write(STDERR_FILENO, error_message, strlen(error_message));
  }
}

// A fn that is used to overwrite the existing path.
void set_path(char **args)
{
  if (args[1] == NULL)
  { // If no arguments are passed, set path to be empty.
    path = "";
  }
  else
  {
    path = ""; // Reset the path to overwrite.
    int i = 1;
    while (args[i] != NULL)
    {
      if (args[i] == NULL)
      {
        break;
      }
      else
      { // Paths are separated by colons (:).
        strcat(path, strcat(":", args[i]));
      }
      ++i;
    }
  }
}

// DOES NOT WORK!
// A fn that takes a line of input and splits
// it into distinct commands.
// char** parse_input(char *line)
// {
//   // Separate the argument into commands.
//   char *cmd_token = strtok(line, "&");
//   // Separate the commands into their individual arguments.
//   char **parsed_cmds; // String tokens within tokens.
//   int i = 0;
//   while (cmd_token != NULL)
//   {
//     parsed_cmds[i++] = strtok(cmd_token, " ");
//     cmd_token = strtok(NULL, "&");
//   }

//   return **parsed_cmds;
// }

// A fn that takes a string command and
// breaks it up into its individual arguments.
char** parse_cmd(char *cmd_arg_token)
{
  int num_args = 8; // Number of arguments in the command.
  char **cmd_arr = malloc(num_args * sizeof(char *)); // An array of strings.
  int i = 0;
  while (cmd_arg_token != NULL)
  {
    cmd_arr[i++] = cmd_arg_token;
    cmd_arg_token = strtok(NULL, " ");

    if (i == num_args-1)
    { // If array becomes full, double the memory size of the array.
      num_args *= 2;
      cmd_arr = realloc(cmd_arr, num_args * sizeof(char *));
    }
  }

  return cmd_arr;
}

// A fn that launches new processes.
int launch_process(char **args)
{
  pid_t pid;
  int status;

  pid = fork();
  if (pid == 0)
  { // Child process.
    if (execv(path, args) == -1)
    {
      perror("grsh"); // Forking error.
    }
    exit(EXIT_FAILURE);
  } // Fork failed.
  else if (pid < 0)
  {
    perror("grsh"); // Forking error.
  }
  else
  { // Parent process.
    do
    {
      waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

// A fn used to execute a single command.
int execute_cmd(char **cmd_arr)
{
  int i;

  if (cmd_arr[0] == NULL)
  { // No command was passed.
    return 1;
  }

  // Check if the command is one of the builtin commands: exit, cd, path
  // If yes, call the corresponding builtin function for that command.
  for (i = 0; i < sizeof(builtin_cmds)/sizeof(builtin_cmds[0]); ++i)
  {
    if (strcmp(cmd_arr[0], builtin_cmds[i]) == 0)
    {
      (*builtin_cmd_funcs[i])(cmd_arr);
      return 0;
    }
  }

  return launch_process(cmd_arr);
}

// A fn used to execute multiple commands in parallel.
int execute_cmds(char *cmds)
{
  char *cmd_token = strtok(cmds, "&"); // Split line into distinct commands.
  char *cmd_arg_token = strtok(cmd_token, " "); // Split command into its args.

  while (cmd_token != NULL)
  {
    char **cmd_arr = parse_cmd(cmd_arg_token);
    execute_cmd(cmd_arr);
    free(cmd_arr);
    cmd_token = strtok(NULL, "&");
  }

  return 0;
}

// In interactive mode, the shell provides a shell
// prompt, waits for the user to enter some commands,
// and then the shell executes them.
void interactive_mode()
{
  size_t len = 0;      // Length of the input line.
  ssize_t read;        // Read status of the input line.
  char *line = NULL;   // The input line.
  //char **cmds;

  printf("grsh> ");
  while ((read = getline(&line, &len, stdin)) != -1)
  {
    //cmds = parse_input(line);
    execute_cmds(line);
  }

  exit(0);
}

// In batch mode, the shell reads commands from a batch file and executes them.
void batch_mode(char *filename)
{
  size_t len = 0;      // Length of the input line.
  ssize_t read;        // Read status of the input line.
  char *line = NULL;   // The input line.
  //char **cmds;
  FILE *fptr = fopen(filename, "r"); // Open file in read mode.

  while((read = getline(&line, &len, fptr)) != -1)
  {
    //cmds = parse_input(line);
    execute_cmds(line);
  }

  exit(0);
}

// Program execution starts here.
int main(int argc, char *argv[])
{
  // If no arguments are passed, use interactive mode.
  // Else use batch mode with the specified batch file.
  if (argc < 2)
  {
    interactive_mode();
  }
  else
  {
    batch_mode(argv[1]);
  }

  return 0;
}