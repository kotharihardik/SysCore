# POSIX Shell Implementation – Assignment 2 (Advanced Operating Systems)

##  Overview
 
The shell supports **foreground and background process management**, **I/O redirection**, **piping**, **autocomplete**, **command history**, and several built-in commands.  
It mimics the behavior of a real shell while handling errors gracefully.  

The implementation is done in a **modular fashion**, separating concerns into multiple files for readability, maintainability, and extensibility.  

---

##  How to Run the Project

### 1. Compile
The project uses a `makefile` for compilation:
```bash
make
```
This generates the shell executable (e.g., `./mysh`).

### 2. Run the Shell
```bash
./mysh
```

### 3. Exit the Shell
Use:
```bash
exit
```
or press **CTRL-D**.

---

##  Features Implemented

### 1. Shell Prompt
- Displays prompt in the format:
  ```
  <username>@<system_name>:<current_directory>$
  ```
- Shows `~` for the home directory.
- Handles extra spaces/tabs gracefully.

### 2. Built-in Commands
- **`cd`** → Supports `.`, `..`, `-`, `~` with error handling.  
- **`pwd`** → Prints absolute working directory.  
- **`echo`** → Prints arguments as-is, handling spaces/tabs.  
- **`ls`** → Supports `-a`, `-l`, `-al/-la`, with file/directory arguments.  
- **`pinfo`** → Prints process info of shell or given PID.  
- **`search`** → Recursively searches for a file/folder inside a given path.  

### 3. Process Management
- **Foreground execution** → Waits until process finishes.  
- **Background execution (`&`)** → Prints PID, shell continues running.  
- Supports **multiple background jobs**.  

### 4. I/O Redirection
- Supports `<`, `>`, `>>`.  
- Creates output files with `0644` permissions.  
- Handles errors for missing files.  

### 5. Piping
- Supports **multiple pipes (`|`)**.  
- Works with redirection inside pipelines.  

### 6. Signal Handling
- **CTRL-Z** → Pushes foreground job to background (stopped).  
- **CTRL-C** → Interrupts current foreground process.  
- **CTRL-D** → Exits the shell gracefully.  

### 7. Autocomplete
- **TAB-based completion** for commands and files.  
- Works with single and multiple matches.  

### 8. Command History
- Stores last **20 commands** across sessions.  
- **`history`** → Shows last 10 commands.  
- **`history <n>`** → Shows last _n_ commands (n ≤ 20).  
- Supports **↑ / ↓ arrow keys** to browse history.  

---

##  Project Structure

```text
2025201046_Assignment2/
├── README.md          # Documentation and usage guide
├── Makefile           # Build script for compiling the shell
├── header.h           # Function declarations and shared definitions
├── main.cpp           # Entry point of the shell (main loop, signal handling)
├── builtins.cpp       # Built-in commands (cd, pwd, echo, etc.)
├── externals.cpp      # External command handling (ls, pinfo, search, etc.)
├── history.cpp        # Command history management (load, save, display)
├── parsers.cpp        # Tokenizer and parser utilities (command parsing)
├── pipeline.cpp       # Pipeline handling (command1 | command2 | ...)
└── utils.cpp          # I/O redirection and helper utilities

```

##  System Calls and Functions Used

```c
fork()        // Creates a new child process
execvp()      // Executes external programs inside a process
waitpid()     // Waits for a child process to change state (used for foreground jobs)
kill()        // Sends signals to processes (e.g., terminate, stop)

getcwd()      // Retrieves current working directory
chdir()       // Changes the current working directory
opendir()     // Opens a directory for reading
readdir()     // Reads directory entries
closedir()    // Closes an opened directory
stat()        // Gets file/directory information (used in ls -l)

open()        // Opens/creates files for I/O redirection
close()       // Closes a file descriptor
dup2()        // Duplicates a file descriptor (used for pipes and redirection)

pipe()        // Creates a unidirectional communication channel between processes
dup()         // Duplicates an existing file descriptor

signal()      // Registers a handler for signals like SIGINT, SIGTSTP
getpid()      // Retrieves the process ID of the calling process
gethostname() // Retrieves system/host name
getenv()      // Retrieves environment variables

strtok()      // Tokenizes user input based on delimiters
perror()      // Prints descriptive error messages from system errors
```


## 🛠️ Approach

1. **Parsing Input**  
   - Input is tokenized using `strtok`.  
   - Handles special operators: `;`, `|`, `<`, `>`, `>>`, `&`.  

2. **Built-in Commands**  
   - Implemented `cd`, `pwd`, `echo`, `ls`, `search`, and `pinfo`.  
   - For **`pinfo`**, since macOS does not have `/proc`, we used `sysctl` and `libproc` to fetch process details such as PID, status (R, S, T, Z, I), memory usage, and executable path.  
   - For **`search`**, recursive directory traversal is implemented using `opendir()` and `readdir()`. It skips `.` and `..`, checks subdirectories, and returns `true` if the target file/folder is found.  

3. **Process Management**  
   - Uses `fork()`, `execvp()`, and `waitpid()`.  
   - Foreground processes block until completion, background processes continue with PID printed.  

4. **Pipes & Redirection**  
   - Implemented with `pipe()`, `dup2()`, and file descriptors.  
   - Works with multiple commands chained using `|`.  
   - Supports input (`<`), overwrite (`>`), and append (`>>`) redirection.  

5. **Signal Handling**  
   - Custom handlers for `SIGINT` and `SIGTSTP` (CTRL-C and CTRL-Z).  
   - Foreground jobs can be stopped and pushed to background.  

6. **Autocomplete**  
   - Uses directory scanning to suggest commands/files when TAB is pressed.  

7. **History**  
   - Stores last 20 commands persistently in a history file.  
   - Commands can be recalled using `history` or arrow keys.  



---

##  Author
**Hardik Kothari**  
2025201046
