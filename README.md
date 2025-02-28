# Shell

This is a simple shell implemented in C that supports basic command execution, variable handling, and background processes. It demonstrates core shell functionalities with a custom command parser and doubly linked list for token management.

---

## Features

- **Built-in Commands:**
  - **`cd [directory]`**: Change the current working directory.
  - **`pwd`**: Display the current working directory.
  - **`echo [string]`**: Output a string to the console.
  - **`export [identifier] = [value]`**: Set environment variables for later use.
  - **`exit`**: Terminate the shell.

- **External Command Execution:**
  - Executes non-built-in commands using `fork()` and `execvp()`.
  
- **Background Process Execution:**
  - Append an ampersand (`&`) at the end of a command to execute it in the background.
  - Uses a SIGCHLD handler to notify when background processes terminate.

- **Variable Expansion:**
  - Tokens starting with `$` are expanded using values from previously exported variables.

- **Custom Command Parsing:**
  - Uses a doubly linked list to tokenize and process input.
  - Supports quoted strings for complex inputs (e.g., in `export`).

---

## Getting Started

### Prerequisites

- A C compiler (e.g., `gcc`)
- A POSIX-compliant operating system

### Compilation

Compile the shell with:
```bash
gcc -o shell shell.c
```
This command compiles the source code into an executable named `shell`.

### Running the Shell

Start the shell by running:
```bash
./shell
```
After launching, the shell will wait for your input. Commands can be entered as you would in any typical Unix-like shell.

---

## Usage Examples

- **Changing Directory:**
  ```bash
  cd /path/to/directory
  ```

- **Printing the Working Directory:**
  ```bash
  pwd
  ```

- **Echoing a String:**
  ```bash
  echo "Hello, World!"
  ```

- **Exporting a Variable:**
  ```bash
  export MY_VAR = "some value"
  ```
  Then, use the variable:
  ```bash
  echo $MY_VAR
  ```

- **Executing an External Command in the Background:**
  ```bash
  some_command &
  ```

- **Exiting the Shell:**
  ```bash
  exit
  ```

---

## Code Structure Overview

- **Environment Setup:**
  - The shell starts by changing to the root directory and setting up a SIGCHLD handler to manage background processes.

- **Command Parsing:**
  - Input is read line-by-line.
  - A custom parser tokenizes the input using a doubly linked list.
  - Special handling exists for commands like `export` (to handle variable assignment and quoted strings) and `echo` (to capture the entire following string).

- **Variable Management:**
  - Variables are stored in a linked list.
  - When a token starts with `$`, it is replaced with the corresponding exported variable's value.

- **Command Execution:**
  - Built-in commands (e.g., `cd`, `pwd`, `echo`, `export`, `exit`) are handled directly.
  - Other commands are executed via `fork()` and `execvp()`.
  - The shell supports background execution by checking for the `&` symbol.

- **Memory Management:**
  - The implementation uses dynamic memory allocation for both the linked list and command arguments.
  - Proper cleanup is performed to free allocated memory after command execution.

---

## Limitations & Future Improvements

- **Error Handling:**  
  Current error handling is basic. Future improvements could include more robust error checking and informative messages.

- **Feature Enhancements:**  
  Future versions might add support for:
  - Command piping and redirection.
  - Command history.
  - Advanced variable handling.
  - Improved parsing for more complex shell functionalities.

---

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
