#ifndef SPEAKER_H
#define SPEAKER_H

#include <stdint.h>

void sound_beep(uint32_t frequency, uint32_t duration_ms);
void sound_set_enabled(int enabled);

#endif