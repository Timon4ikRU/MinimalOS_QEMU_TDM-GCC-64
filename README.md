**!!!CHANGE PATH TO QEMU IN BUILD.BAT FIRST!!!**

# 🖥️ Bare-Metal Operating System

A custom 32-bit operating system written from scratch in **C** and **Assembly** for the x86 architecture. Designed to run directly on bare-metal hardware or inside emulators such as QEMU.

It features basic I/O drivers, a custom FAT filesystem, an integrated text editor, and PC Speaker sound support.

---

## ✨ Key Features

- **Video Output (VGA Text Mode):** Text rendering with configurable console colors managed via a system registry.
- **Keyboard Input (PS/2 Keyboard):** Raw scancode handling, support for control keys (arrow navigation, Backspace, Enter), and key bindings (`Ctrl+S`, `Ctrl+Q`, `Ctrl+C`, `Alt+Shift+J`).
- **Sound Driver (PC Speaker / PIT):**
  - Hardware frequency audio output with configurable duration.
  - Global audio toggle via system registry (`SOUND=1` / `SOUND=0`).
  - Built-in melody playback command (`PLAY`).
- **Filesystem Support (FAT):**
  - File and directory operations (`CF`, `CDIR`, `CD`, `RD`, `RF`, `LS`/`DIR`).
  - Relative and absolute directory navigation.
- **NANO Text Editor:**
  - Built-in console text editor.
  - Dual modes: Insert (`INSERT`) and Overwrite (`REPLACE`).
  - Full arrow key navigation (**UP**, **DOWN**, **LEFT**, **RIGHT**).
  - Save to disk shortcut (`Ctrl+S`).
- **System Registry (`sysmain.registry`):**
  - Persistent system configuration (UI colors `COLOR=0xXX`, sound toggle `SOUND=1`/`SOUND=0`).

---

## 🛠️ Building & Running

### Prerequisites

To build and run the operating system, you will need:
- **GCC** (cross-compiler `i686-elf-gcc` or standard `gcc` with 32-bit support via `-m32`)
- **NASM** (Netwide Assembler)
- **GNU LD** (Linker)
- **QEMU** (`qemu-system-i386` or `qemu-system-x86_64`)

### Building the Project

```bash
# Example compilation and linking process
nasm -f elf32 boot.asm -o boot.o
gcc -m32 -c kernel.c -o kernel.o -ffreestanding -O2 -Wall
gcc -m32 -c drv/speaker.c -o speaker.o -ffreestanding -O2 -Wall
# ... compile remaining driver files

ld -m elf_i386 -T linker.ld -o os-kernel.bin boot.o kernel.o speaker.o

```

### Running in QEMU

```bash
qemu-system-i386 -kernel os-kernel.bin -soundhw pcspk

```

*(The `-soundhw pcspk` or `-machine pcspk-audiodev=...` flag enables PC Speaker emulation).*

---

## 💻 CLI Commands

| Command | Description |
| --- | --- |
| `HELP` | Display available system commands |
| `LS` / `DIR` | List directory contents |
| `CD <path>` | Change directory (`..` to navigate up) |
| `CF <file>` | Create an empty file |
| `RF <file>` | Remove a file |
| `CDIR <path>` | Create a new directory |
| `RD <path>` | Remove a directory |
| `NANO <file>` | Open a file in the NANO text editor |
| `PLAY` | Play a test melody through the PC Speaker |
| `CLS` | Clear the terminal screen |

---

## 🎹 NANO Editor Shortcuts

* **Arrow Keys (↑, ↓, ←, →)** — Cursor navigation across lines.
* **Alt + Shift + J** — Toggle between `INSERT` and `REPLACE` modes.
* **Ctrl + S** — Save current file to disk.
* **Ctrl + Q** / **Ctrl + C** — Exit editor without saving.

---

## 📄 License

This project is licensed under the **MIT License**. See the `LICENSE` file for details.
