/*
 * Filename: grsh.c
 * Author: Akash Kumar
 * Class: CSC 331
 * Date: 12/05/2019
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>

#define DELIMITERS " \t\v\n\r"

// Builtin functions:
void exit_grsh(char **args);
void cd(char **args);
void set_path(char **args);

// Global variables:
const char *builtin_cmds[3] = {"exit","cd","path"}; // array of strings.
const void (*builtin_cmd_funcs[]) (char **) = {&exit_grsh, &cd, &set_path}; // array of pointers to functions.
const char error_message[30] = "An error has occurred\n";
char path[16384] = "/bin"; // Default path to executables.
char *cmd[20] = {[0 ... 19] = NULL}; // command to be executed.
int num_args = 0; // Number of arguments in cmd.

// A debugging fn used to print a string array.
void print_string_array(char **arr)
{
  static int calls = 0; // number of times the function gets called.
  calls++;
  for (int i = 0; i < sizeof(arr)/sizeof(arr[0]); ++i)
  {
    printf("cmd array, call %d: [%s]\n", calls, arr[i]);
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
  }
  if (chdir(cd_path) != 0)
  {
    write(STDERR_FILENO, error_message, strlen(error_message));
  }
}

// A fn that is used to overwrite the existing path.
void set_path(char **args)
{
  if (args[1] == NULL)
  { // If no arguments are passed, set path to be empty.
    memset(path, 0, sizeof(path)); //path = "";
  }
  else
  {
    memset(path, 0, sizeof(path)); //path = ""; // Reset the path to overwrite.
    strcpy(path, args[1]);
    int i = 2;
    while (args[i] != NULL)
    { // Paths are separated by colons (:).
      strcat(path, ":");
      strcat(path, args[i]);
      ++i;
    }
  }
}

// A fn that launches new processes.
int launch_process()
{
  pid_t pid;
  char exec_path[4096]; // Path to the executable file.
  char *save_ptr;
  char *path_token;

  path_token = strtok_r(strdup(path), ":", &save_ptr);
  while(path_token != NULL)
  {
    strcpy(exec_path, path_token);
    strcat(exec_path, "/");
    strcat(exec_path, cmd[0]);
    if (access(exec_path, X_OK) == 0)
    { //printf("Access ok!\n");
      break;
    }

    memset(exec_path, 0, sizeof(exec_path)); // exec_path = "";
    path_token = strtok_r(NULL, ":", &save_ptr);
  }

  pid = fork();
  if (pid < 0)
  { // Fork failed.
    write(STDERR_FILENO, error_message, strlen(error_message));
    exit(EXIT_FAILURE);
  }
  else if (pid == 0)
  { // Child process.
    execv(exec_path, cmd); // Execute the command.
    // If execv returns, we know the command failed to execute.
    write(STDERR_FILENO, error_message, strlen(error_message));
    exit(EXIT_FAILURE);
  }
  else
  { // Parent process.
    int status;
    do
    {
      waitpid(pid, &status, WUNTRACED);
    } while (!WIFEXITED(status) && !WIFSIGNALED(status));
  }

  return 1;
}

// A fn used to execute a single command.
int execute_cmd()
{
  if (num_args == 0)
  { // No command was passed.
    return 0;
  }

  // Check if the command is one of the builtin commands: exit, cd, path
  // If yes, call the corresponding builtin function for that command.
  for (int i = 0; i < sizeof(builtin_cmds)/sizeof(builtin_cmds[0]); ++i)
  {
    if (strcmp(cmd[0], builtin_cmds[i]) == 0)
    {
      (*builtin_cmd_funcs[i])(cmd);
      return 1; // Execution success.
    }
  }

  return launch_process();
}

// A fn used to execute multiple commands in parallel.
int execute_cmds(char *input)
{
  int exec_status = 0; // bool for status of execution. 0 = False, 1 = True.
  int output, error, save_output, save_error; // For redirection.
  char *save_ptr;
  char *token;

  token = strtok_r(strdup(input), DELIMITERS, &save_ptr);
  while (token != NULL)
  {
    if (strcmp(token, "&") == 0) // Parallel command execution.
    {
      exec_status = execute_cmd();
    }
    else if (strcmp(token, ">") == 0) // Redirection.
    {
      token = strtok_r(NULL, DELIMITERS, &save_ptr); // Filename.
      // Open output file.
      output = open(token, O_RDWR|O_CREAT|O_APPEND, 0600);
      if (output == -1)
      {
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(EXIT_FAILURE);
      }
      // Open error file.
      error = open(token, O_RDWR|O_CREAT|O_APPEND, 0600);
      if (error == -1)
      {
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(EXIT_FAILURE);
      }

      // Save copies of the file descriptors.
      save_output = dup(fileno(stdout));
      save_error = dup(fileno(stderr));

      // Redirect the outputs and errors to stdout and stderr.
      if (dup2(output, fileno(stdout)) == -1)
      {
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(EXIT_FAILURE);
      }
      if (dup2(error, fileno(stdout)) == -1)
      {
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(EXIT_FAILURE);
      }

      exec_status = execute_cmd();

      // Flush the streams and close them.
      fflush(stdout); close(output);
      fflush(stderr); close(error);

      // Reset to the original file descriptors.
      dup2(save_output, fileno(stdout));
      dup2(save_error, fileno(stderr));

      // Close the file descriptor copies.
      close(save_output);
      close(save_error);
    }
    else // Build command.
    {
      cmd[num_args++] = token;
    }

    token = strtok_r(NULL, DELIMITERS, &save_ptr); // Advance to next token.

    // Cleanup after command execution.
    if (exec_status)
    {
      exec_status = 0; // Reset bool.
      memset(cmd, 0, sizeof(cmd)); // Clear command arguments.
      num_args = 0;
    }
  }

  // Execute the last command.
  if (num_args > 0)
  {
    exec_status = execute_cmd();
  }

  // Cleanup after command execution.
  if (exec_status)
  {
    exec_status = 0; // Reset bool.
    memset(cmd, 0, sizeof(cmd)); // Clear command arguments.
    num_args = 0;
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

  printf("grsh> ");
  while ((read = getline(&line, &len, stdin)) != -1)
  {
    execute_cmds(line);
    printf("grsh> ");
  }
}

// In batch mode, the shell reads commands from a batch file and executes them.
void batch_mode(char *filename)
{
  size_t len = 0;      // Length of the input line.
  ssize_t read;        // Read status of the input line.
  char *line = NULL;   // The input line.
  FILE *fptr = fopen(filename, "r"); // Open file in read mode.

  if (!fptr)
  {
    write(STDERR_FILENO, error_message, strlen(error_message));
    exit(1);
  }

  while((read = getline(&line, &len, fptr)) != -1)
  {
    execute_cmds(line);
  }

  fclose(fptr);
}

// Program execution starts here.
int main(int argc, char *argv[])
{ // If no arguments are passed, use interactive mode.
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
