#include "idt.h"

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

extern void idt_load(uint32_t);
extern void irq1_handler(void);

struct idt_entry idt[256];
struct idt_ptr idtp;

void idt_set_gate(uint8_t num, uint32_t base, uint16_t selector, uint8_t flags) {
    idt[num].base_low = base & 0xFFFF;
    idt[num].base_high = (base >> 16) & 0xFFFF;
    idt[num].selector = selector;
    idt[num].zero = 0;
    idt[num].flags = flags;
}

// Remapping PIC interrupts controller (IRQ0-7 -> 0x20-0x27)
static void pic_remap(void) {
    outb(0x20, 0x11);
    outb(0xA0, 0x11);
    outb(0x21, 0x20); // Master PIC offset (0x20)
    outb(0xA1, 0x28); // Slave PIC offset (0x28)
    outb(0x21, 0x04);
    outb(0xA1, 0x02);
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    
    // Masking all interrupts except IRQ1 (keyboard)
    outb(0x21, 0xFD); // 0xFD = 1111 1101b (Only IRQ1 is turned on)
    outb(0xA1, 0xFF);
}

void idt_init(void) {
    idtp.limit = (sizeof(struct idt_entry) * 256) - 1;
    idtp.base = (uint32_t)&idt;

    // Clearing the table
    for (int i = 0; i < 256; i++) {
        idt_set_gate(i, 0, 0, 0);
    }

    pic_remap();

    // Registering keyboard handler to the vector 0x21 (IRQ1)
    // 0x08 = Kernel Code Segment, 0x8E = Present, Ring 0, Interrupt Gate
    idt_set_gate(0x21, (uint32_t)irq1_handler, 0x08, 0x8E);

    // Loading IDT
    idt_load((uint32_t)&idtp);

    // Allowing interrupts (sti)
    asm volatile ("sti");
}