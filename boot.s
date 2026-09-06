.code 16
.section .text
.global _start

_start:
    # Saving disk number
    movb %dl, (BOOT_DRIVE)

    # Setting up segments
    cli
    xorw %ax, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %ss
    movw $0x9000, %sp
    sti

    # Resetting disk controller before reading
    xorw %ax, %ax
    movb (BOOT_DRIVE), %dl
    int $0x13

    # Reading the core (reading 8 sectors from the 2nd sector)
    movw $0x1000, %bx
    movb $8, %dh
    movb (BOOT_DRIVE), %dl
    call disk_load

    # Going into the 32-bit Protected Mode
    cli
    lgdt (gdt_descriptor)

    movl %cr0, %eax
    orb $1, %al
    movl %eax, %cr0

    # Far jump into the 32-bit code
    ljmp $0x08, $init_pm

disk_load:
    pusha
    movb $0x02, %ah
    movb %dh, %al
    movb $0x00, %ch
    movb $0x00, %dh
    movb $0x02, %cl
    int $0x13
    jc disk_error
    popa
    ret

disk_error:
    # If there's an error - just halt
    hlt

# --- GDT Table ---
.align 8
gdt_start:
gdt_null:
    .long 0x0, 0x0
gdt_code:
    .word 0xffff, 0x0000
    .byte 0x00, 0x9a, 0xcf, 0x00
gdt_data:
    .word 0xffff, 0x0000
    .byte 0x00, 0x92, 0xcf, 0x00
gdt_end:

gdt_descriptor:
    .word gdt_end - gdt_start - 1
    .long gdt_start

.code32
init_pm:
    movw $0x10, %ax
    movw %ax, %ds
    movw %ax, %ss
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs

    movl $0x90000, %esp
    movl %esp, %ebp

    # Jumping to kernel_entry
    jmp 0x1000

BOOT_DRIVE: .byte 0

.org 510
.word 0xaa55
