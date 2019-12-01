/*
 * Filename: grsh.c
 * Author: Akash Kumar
 * Class: CSC 331
 * Date: 11/30/2019
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
//char *path = "/bin"; // Default path to executable commands.
char path[1024] = "/bin"; // Default path to executable commands.
const char error_message[30] = "An error has occurred\n";
const char error_message1[30] = "An erro1 has occurred\n";
const char error_message2[30] = "An erro2 has occurred\n";

// A test fn used to print a string array.
void print_string_array(char **arr)
{
  for (int i = 0; i <= sizeof(arr)/sizeof(arr[0]); ++i)
  {
    printf("args array: [%s]\n", arr[i]);
  }
}

// A fn that is used to exit the shell.
void exit_grsh(char **args)
{
  exit(0);
}

// A fn that is used to change directories.
void cd(char **args)
{
  char *cd_path = args[1];
  if (cd_path == NULL)
  {
    write(STDERR_FILENO, error_message, strlen(error_message));
    exit(1);
  }
  int status = chdir(cd_path);
  if (status != 0)
  { //printf("This should not print %d", status);
    write(STDERR_FILENO, error_message, strlen(error_message));
    exit(1);
  }
}

// A fn that is used to overwrite the existing path.
void set_path(char **args)
{
  //printf(args[1]);
  if (args[1] == NULL)
  { // If no arguments are passed, set path to be empty.
    memset(path, 0, sizeof(path)); //path = "";
  }
  else
  { //printf("in else");
    memset(path, 0, sizeof(path)); //path = ""; // Reset the path to overwrite.
    int i = 1;
    if (args[i] != NULL)
    {
      strcat(path, args[i]);
      ++i;
    }
    while (args[i] != NULL)
    {
      // Paths are separated by colons (:).
      // path = realloc(path, strlen(path) + strlen(":") + strlen(args[i]));
      strcat(path, ":");
      strcat(path, args[i]);

      // char colon = ':';
      // if(strcmp(path, "") != 0)
      // {
      //   strncat(path, &colon, 1);
      // }
      // char *arg = args[i];

      // for (int j = 0; j < sizeof(arg)/sizeof(arg[0]); ++j)
      // {
      //   strncat(path, &arg[j], 1);
      // }

      ++i;
    }
    //printf("%s\n", path);
  }
}

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
    cmd_arg_token = strtok(NULL, " \t\n");

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
  print_string_array(args); printf("path: %s\n", path);

  pid = fork(); printf("ak1");
  if (pid == 0)
  { // Child process.
    printf("ak2");
    if (execv(path, args) == -1)
    { printf("ak3");
      write(STDERR_FILENO, error_message1, strlen(error_message1)); // Forking error.
    }
    exit(EXIT_FAILURE);
  } // Fork failed.
  else if (pid < 0)
  {
    write(STDERR_FILENO, error_message, strlen(error_message)); // Forking error.
  }
  else
  { // Parent process.
    do
    { printf("ak5");
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
  { //printf("in for");
    if (strcmp(cmd_arr[0], builtin_cmds[i]) == 0)
    { //printf("in if\n");
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

  while (cmd_token != NULL)
  {
    char *cmd_arg_token = strtok(cmd_token, " \t\n"); // Split command into its args.
    char **cmd_arr = parse_cmd(cmd_arg_token);
    //print_string_array(cmd_arr);
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
    printf("grsh> ");
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