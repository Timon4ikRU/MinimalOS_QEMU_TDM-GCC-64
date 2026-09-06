.code32
.global _idt_load
.global idt_load
.global _irq1_handler
.global irq1_handler

.extern _keyboard_handler
.extern keyboard_handler

# Loading IDT into the processor
_idt_load:
idt_load:
    movl 4(%esp), %eax
    lidt (%eax)
    ret

# IRQ1 interrupt handler (keyboard)
_irq1_handler:
irq1_handler:
    pusha                   # Saving registers

    call _keyboard_handler  # Calling a C function (underscore is for TDM-GCC)

    popa                    # Setting registers back to what we saved
    iret                    # Returning from the interrupt
