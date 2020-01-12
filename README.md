# Golden Ram Shell
 A simple C implementation of a Linux shell.
 ![Grsh Shell Demonstration](https://github.com/KumarUniverse/Golden-Ram-Shell/blob/master/grsh-shell-example.png)

## Built-in Commands
- **exit** - used to exit the grsh shell.
- **cd** - used to change directories.
- **path** - used to set the search path of the shell.

## Supports
- Linux commands like `ls`, `cat`, `grep`, `echo`, `cal`, etc.
- Parallel execution
- Redirection
- Interactive mode
- Batch mode

## Key C Functions Used
- access()
- fork()
- execv()
- waitpid()
- exit()
- chdir()
- strtok_r()
