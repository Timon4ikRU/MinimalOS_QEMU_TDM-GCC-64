#include "speaker.h"

static int sound_enabled = 1;

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline void outb(uint16_t port, uint8_t val) {
    asm volatile ("outb %0, %1" : : "a"(val), "Nd"(port));
}

void sound_set_enabled(int enabled) {
    sound_enabled = enabled;
}

// Delay in milliseconds
static void pit_delay(uint32_t duration_ms) {
    for (uint32_t m = 0; m < duration_ms; m++) {
        for (volatile int i = 0; i < 4000; i++) {
            inb(0x80); // Hardware delay step
        }
    }
}

void sound_beep(uint32_t frequency, uint32_t duration_ms) {
    if (!sound_enabled) return;

    if (frequency == 0) {
        uint8_t tmp = inb(0x61) & 0xFC;
        outb(0x61, tmp);
        pit_delay(duration_ms);
        return;
    }

    uint32_t div = 1193180 / frequency;
    outb(0x43, 0xB6);
    outb(0x42, (uint8_t)(div & 0xFF));
    outb(0x42, (uint8_t)((div >> 8) & 0xFF));

    uint8_t tmp = inb(0x61);
    if ((tmp & 3) != 3) {
        outb(0x61, tmp | 3);
    }

    pit_delay(duration_ms);

    // Turning the speaker off on sound end
    tmp = inb(0x61) & 0xFC;
    outb(0x61, tmp);
}