
# msh - Mini Shell

A simple Unix-like shell written in C that supports basic command execution, command history with circular buffer, and built-in commands.

## Features

- Execute external commands (e.g., `ls`, `pwd`, `echo`)
- Built-in commands:
  - `cd <dir>` — Change directory
  - `exit` / `quit` — Exit the shell
  - `history` — Show command history (up to 14 entries)
  - `!<n>` — Re-execute the nth command in history
- Circular command history buffer (size 14)
- Search for commands in:
  - Current directory
  - `/usr/local/bin`
  - `/usr/bin`
  - `/bin`

## Getting Started

### Build

```bash
gcc msh.c -o msh
```

### Run

```bash
./msh
```

### Example Usage

```shell
msh  /home/user> ls
Documents  Downloads  msh.c

msh  /home/user> cd Downloads

msh  /home/user> !0
ls

msh  /home/user> history
 0: ls
 1: cd Downloads
 2: !0
```

## Notes

- Only the first 5 tokens (command + 4 arguments) are parsed.
- History is stored in memory (not persistent across runs).
- Invalid commands will return a “Command not found” message.
- `!n` refers to the index shown in the `history` command (not necessarily the command number across sessions).
