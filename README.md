# 🐚 MyShell – A Custom Unix Shell

[![C](https://img.shields.io/badge/language-C-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)
[![Platform](https://img.shields.io/badge/platform-Linux%20%7C%20Unix-lightgrey)]()

A lightweight, interactive shell written in C that supports user‑defined variables, background execution, and built‑in commands like `cd`, `export`, `echo`, and `exit`. Perfect for learning how shells work under the hood.

## ✨ Features

- **Built‑in commands** – `cd`, `exit`, `export`, `echo`
- **Variable substitution** – Define variables with `export NAME=VALUE` and use them with `$NAME`
- **Background execution** – Run commands with `&`; the shell logs their termination to `shell.log`
- **Signal handling** – `SIGINT` (Ctrl+C) is ignored by the shell but passed to foreground child processes  
  `SIGCHLD` reaps background processes automatically
- **Coloured prompt** – Shows the current working directory in red

## 📦 Getting Started

### Prerequisites

- A Unix‑like operating system (Linux, macOS, WSL)
- GCC or any C compiler

### Compilation

Clone the repository and compile the source:

```bash
git clone https://github.com/yourusername/myshell.git
cd myshell
gcc -o myshell myshell.c

Running
bash

./myshell

You will see a prompt like:
text

/home/user>
```


📂 File Structure
text
```bash
myshell/
├── myshell.c          # Main source code
├── shell.log          # Created automatically – logs background process exits
└── README.md          # This file
```
⚠️ Limitations

    Variable names must be alphanumeric (no ${} or concatenation like $NAMEworld)

    export does not allow spaces around = (e.g., export NAME = value is invalid)

    Maximum 100 variables, 100 background PIDs, 1000 characters per input line

    No support for pipes (|) or redirections (>, <)

    echo does not interpret escape sequences like \n

🛠️ How It Works 

    Prompt & Input – The shell displays the current directory and reads a line.

    Parsing – The input is split into tokens (arguments).

    Built‑in Handling – Commands like cd, export, echo, exit are executed directly.

    Variable Substitution – Any token starting with $ is replaced by the variable’s value.

    Fork & Exec – For external commands, the shell forks a child process and calls execvp().

    Background / Foreground – If & is present, the parent does not wait; otherwise it waits for the child.

    Signal Handling – SIGCHLD reaps background children and logs their termination.
