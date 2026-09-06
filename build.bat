@echo off
chcp 65001 > nul

if not exist data mkdir data
if not exist data\sys mkdir data\sys
if not exist data\sys\sysmain.registry echo COLOR=0x0F > data\sys\sysmain.registry

:: 1. Building ASM
as --32 kernel_entry.s -o kernel_entry.o
if %errorlevel% neq 0 goto error

as --32 interrupts.s -o interrupts.o
if %errorlevel% neq 0 goto error

:: 2. Compiling C files
gcc -m32 -c kernel.c -o kernel.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
if %errorlevel% neq 0 goto error

gcc -m32 -c drv/vga.c -o drv_vga.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
if %errorlevel% neq 0 goto error

gcc -m32 -c drv/keyboard.c -o drv_keyboard.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
if %errorlevel% neq 0 goto error

gcc -m32 -c drv/ata.c -o drv_ata.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
if %errorlevel% neq 0 goto error

gcc -m32 -c drv/idt.c -o drv_idt.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
if %errorlevel% neq 0 goto error

gcc -m32 -c drv/speaker.c -o drv_speaker.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
if %errorlevel% neq 0 goto error

gcc -m32 -c drv/fat.c -o drv_fat.o -std=gnu99 -ffreestanding -O2 -Wall -Wextra
if %errorlevel% neq 0 goto error

:: 3. Linking
ld -m i386pe -T linker.ld kernel_entry.o interrupts.o kernel.o drv_vga.o drv_keyboard.o drv_ata.o drv_idt.o drv_speaker.o drv_fat.o -o kernel.pe
if %errorlevel% neq 0 goto error

objcopy -O binary kernel.pe kernel.bin
if %errorlevel% neq 0 goto error

echo.
echo === Build success! Launching QEMU ===
echo.

path\to\qemu -kernel kernel.bin -drive file=fat:rw:data,format=raw,index=0,media=disk -audiodev dsound,id=snd0 -machine pcspk-audiodev=snd0

goto end

:error
echo.
echo [!] Error while building!

:end
pause