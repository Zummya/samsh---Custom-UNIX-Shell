# samsh - Custom UNIX Shell

A custom command-line shell implementation in C that provides essential UNIX shell functionality including built-in commands, job control, I/O redirection, and command logging.

## Features

### Built-in Commands
- **hop** - Navigate directories with support for `-`, `~`, and `..`
- **reveal** - List directory contents with flags (`-a`, `-l`)
- **log** - Command history management with execution and purging capabilities
- **jobs** - Display background jobs
- **activities** - Show all running processes
- **ping** - Send signals to background processes
- **fg** - Bring background jobs to foreground
- **bg** - Resume stopped jobs in background

### Core Functionality
- **Job Control** - Full support for background (`&`) and foreground processes
- **Signal Handling** - Proper handling of `Ctrl+C` (SIGINT) and `Ctrl+Z` (SIGTSTP)
- **I/O Redirection** - Input (`<`), output (`>`), and append (`>>`) redirection
- **Piping** - Chain commands with pipes (`|`)
- **Command Logging** - Automatic logging of commands to `.shell_log`
- **Custom Prompt** - Dynamic prompt displaying username, hostname, and current directory

## Project Structure

```
.
├── Makefile           # Build configuration
├── include/           # Header files
│   ├── builtins.h    # Built-in command declarations
│   ├── input.h       # Input handling
│   ├── jobs.h        # Job control structures and functions
│   ├── parser.h      # Command parsing
│   ├── prompt.h      # Prompt generation
│   └── shell.h       # Main shell definitions
└── src/              # Source files
    ├── builtins.c    # Built-in command implementations
    ├── input.c       # User input processing
    ├── jobs.c        # Job management
    ├── main.c        # Main shell loop and process execution
    ├── parser.c      # Command line parsing
    └── prompt.c      # Prompt display logic
```

## Building

### Prerequisites
- GCC compiler
- GNU Make
- POSIX-compliant system (Linux/Unix)

### Compilation
```bash
make
```

This will compile the shell with the following flags:
- `-std=c99` - C99 standard
- `-D_POSIX_C_SOURCE=200809L` - POSIX.1-2008 compliance
- `-Wall -Wextra -Werror` - Strict warnings enabled

### Cleaning
```bash
make clean
```

## Usage

### Running the Shell
```bash
./shell.out
```

### Built-in Command Examples

#### hop (change directory)
```bash
hop                    # Go to home directory
hop ~                  # Go to home directory
hop -                  # Go to previous directory
hop /path/to/dir      # Go to specified directory
hop dir1 dir2 dir3    # Navigate through multiple directories
```

#### reveal (list directory)
```bash
reveal                 # List current directory
reveal -a              # Show hidden files
reveal -l              # Long format
reveal -al /path       # Show all files in long format
```

#### log (command history)
```bash
log                    # Show last 15 commands
log purge              # Clear command history
log execute 3          # Execute 3rd command from history
```

#### Job Control
```bash
sleep 100 &            # Run command in background
jobs                   # List background jobs
fg 1                   # Bring job 1 to foreground
bg 1                   # Resume job 1 in background
ping 1 9               # Send signal 9 to job 1
activities             # Show all processes
```

### I/O Redirection
```bash
command < input.txt    # Input redirection
command > output.txt   # Output redirection (overwrite)
command >> output.txt  # Output redirection (append)
```

### Piping
```bash
cat file.txt | grep "pattern" | wc -l
```

## Signal Handling
- **Ctrl+C (SIGINT)** - Terminates foreground process, shell continues
- **Ctrl+Z (SIGTSTP)** - Stops foreground process, adds to job list
- **Ctrl+D (EOF)** - Exits the shell

## Technical Details

### Compiler Flags
- `_POSIX_C_SOURCE=200809L` - Enables POSIX.1-2008 features
- `_XOPEN_SOURCE=700` - Enables X/Open 7 extensions
- `-fno-asm` - Disables asm keyword

### Command Logging
- Commands are stored in `.shell_log` file
- Maximum of 15 commands retained
- Automatically saves after each command execution

### Job Management
- Background jobs tracked with unique job numbers
- States: RUNNING, STOPPED
- Job list maintained as linked list

## License

This project is part of an academic assignment for understanding shell implementations.

## Author

Zummya (samsh---Custom-UNIX-Shell)
