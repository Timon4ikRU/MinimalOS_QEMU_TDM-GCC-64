.code32
.section .text
.global _start
.extern _kernel_main

.align 4
multiboot_header:
    .long 0x1BADB002                  # Magic number
    .long 0x00010000                  # Flags: bit 16 = AOUT kludge
    .long -(0x1BADB002 + 0x00010000)  # Checksum

    # AOUT Kludge (adresses in the memory)
    .long multiboot_header            # Header addr
    .long 0x100000                    # Load addr (1MB)
    .long 0                           # Load end addr
    .long 0                           # BSS end addr
    .long _start                      # Entry point

_start:
    cli
    call _kernel_main
    hlt
