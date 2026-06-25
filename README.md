# CybShell

A Windows command shell built from scratch in C++ using raw Win32 APIs. No wrappers, no libraries — just direct OS-level process management, pipe creation, and I/O redirection.

![CybShell Banner](screenshot.png)

---

## What It Is

Most shell implementations wrap existing system calls or use high-level abstractions. CybShell does neither. Every feature — spawning processes, connecting pipes, redirecting file handles — is implemented directly using the Windows API.

This project was built to understand how a shell actually works at the OS level: what happens when you press enter, how pipes move data between processes, why `cd` has to be a built-in command, and how handle inheritance works.

---

## Features

| Feature | Description |
|---|---|
| PATH Resolution | Type `notepad` instead of `C:\Windows\System32\notepad.exe` |
| Built-in Commands | `cd`, `pwd`, `echo`, `history`, `exit` |
| Pipes | `cmd1 \| cmd2` — connects stdout of cmd1 to stdin of cmd2 |
| Output Redirection | `cmd > file.txt` — saves output to file |
| Append Redirection | `cmd >> file.txt` — appends output to file |
| Input Redirection | `cmd < file.txt` — reads input from file |
| Argument Passing | `notepad file.txt` — passes arguments to external programs |
| Signal Handling | `Ctrl+C` does not kill the shell |
| Command History | `history` shows all commands typed in the session |
| Custom UI | Hacker-aesthetic terminal UI with ANSI color codes |

---

## How to Build

**Requirements:** GCC (MinGW) on Windows

```bash
g++ shell.cpp -o cybshell
```

**Run:**
```bash
./cybshell
```

---

## Code Architecture

The project is split into two files:

- `shell.cpp` — core shell logic
- `shell_ui.h` — terminal UI (colors, banner, prompt)

### shell.cpp — Function Breakdown

#### `get_path_directories()`
Reads the `PATH` environment variable using `getenv("PATH")` and splits it by `;` into a vector of directory strings.

```
PATH = "C:\Windows\System32;C:\Windows;C:\Users\..."
         ↓
["C:\Windows\System32", "C:\Windows", "C:\Users\..."]
```

#### `find_in_path(string command)`
Loops through every directory returned by `get_path_directories()` and checks if the command exists there — first as-is, then with `.exe` appended. Uses `GetFileAttributesA` to check file existence.

```
Input:  "notepad"
Checks: "C:\Windows\System32\notepad"     → not found
        "C:\Windows\System32\notepad.exe" → found
Output: "C:\Windows\System32\notepad.exe"
```

#### `tokenize(string input)`
Splits the user's input by spaces into a vector of tokens. Handles double spaces by skipping empty chunks. `tokens[0]` is always the command name, `tokens[1]` onwards are arguments.

```
Input:  "echo hello world"
Output: ["echo", "hello", "world"]
```

#### `createProcess(string x)`
Wraps `CreateProcessA` to spawn an external program. Uses `WaitForSingleObject` to block until the process exits, then cleans up handles.

#### `execute_pipe(string cmd1, string cmd2)`
Implements pipe execution using `CreatePipe` which creates a read end and a write end:

```
cmd1 stdout → writeEnd → [PIPE BUFFER] → readEnd → cmd2 stdin
```

Key detail: `writeEnd` is closed **before** waiting on `cmd2`. If you don't close it, `cmd2` waits forever for more data that will never come — a deadlock.

Both processes are launched with `TRUE` for handle inheritance so they can access the pipe handles. `SetHandleInformation` marks only the pipe handles as inheritable, preventing other handles from leaking into child processes.

#### I/O Redirection (`>`, `>>`, `<`)
Opens a file using `CreateFileA` with the appropriate flags:

| Operator | Flags |
|---|---|
| `>` | `GENERIC_WRITE` + `CREATE_ALWAYS` |
| `>>` | `GENERIC_WRITE` + `OPEN_ALWAYS` + seek to end |
| `<` | `GENERIC_READ` + `OPEN_EXISTING` |

The file handle is passed into `STARTUPINFOA.hStdOutput` or `hStdInput`, redirecting the process's I/O to the file instead of the terminal.

#### Built-in Commands
Built-ins are handled directly in the main loop without spawning a child process. This is necessary because child processes cannot affect the parent shell's state.

The key example is `cd` — if `cd` spawned a child process, the child would change its own working directory and exit. The shell's directory would remain unchanged. `SetCurrentDirectoryA` is called directly in the shell process instead.

#### Signal Handling
`SetConsoleCtrlHandler` registers a callback that intercepts `Ctrl+C` (`CTRL_C_EVENT`) and returns `TRUE` (handled). This prevents the signal from terminating the shell. Child processes still receive the signal normally.

---

## Code Workflow

```
User types command
        ↓
getline() reads input
        ↓
tokenize() splits into tokens
        ↓
Check tokens[0]:
  ├── "exit"    → break loop
  ├── "cd"      → SetCurrentDirectoryA(tokens[1])
  ├── "pwd"     → GetCurrentDirectoryA()
  ├── "echo"    → print tokens[1...]
  ├── "history" → print history vector
  ├── contains "|"  → execute_pipe(cmd1, cmd2)
  ├── contains ">>" → CreateFileA(OPEN_ALWAYS) + redirect stdout
  ├── contains ">"  → CreateFileA(CREATE_ALWAYS) + redirect stdout
  ├── contains "<"  → CreateFileA(OPEN_EXISTING) + redirect stdin
  └── else → find_in_path(tokens[0]) → createProcess(result + args)
```

---

## Why Certain Decisions Were Made

**Why `cd` is a built-in:**
A child process has its own copy of the working directory. Changing it inside a child has no effect on the parent shell. `cd` must call `SetCurrentDirectoryA` directly in the shell process.

**Why handle inheritance is `TRUE` for pipes:**
`CreateProcessA` needs `TRUE` for inherit handles so that child processes can access the pipe handles passed through `STARTUPINFOA`. Without this, the pipe handles are invalid in the child process.

**Why `writeEnd` is closed before waiting:**
When `cmd2` reads from the pipe, it keeps reading until it sees EOF. EOF is only signaled when all write ends of the pipe are closed. If the shell holds the write end open while waiting for `cmd2`, `cmd2` never sees EOF and hangs forever.

**Why `strncpy` instead of `strcpy`:**
`strcpy` has no bounds checking — writing a command longer than the buffer causes a buffer overflow. `strncpy` with `sizeof(buffer)-1` caps the copy at the buffer size.

---

## Example Usage

```bash
# Run a program
notepad

# Run with arguments
notepad output.txt

# Pipe
cmd /c dir | findstr .cpp

# Save output to file
cmd /c dir > files.txt

# Append to file
cmd /c date /t >> log.txt
cmd /c time /t >> log.txt

# Read from file
findstr shell < files.txt

# Built-ins
cd C:\Users
pwd
echo hello world
history
```

---

## Built With

- C++
- Win32 API (`CreateProcessA`, `CreatePipe`, `CreateFileA`, `SetConsoleCtrlHandler`, `GetCurrentDirectoryA`, `SetCurrentDirectoryA`, `GetFileAttributesA`)
- ANSI escape codes for terminal UI

---

## Author

Taha — [@tahacodes-py](https://github.com/tahacodes-py)

GIKI, Computer Science — Semester 2
