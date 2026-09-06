#ifndef ATA_H
#define ATA_H

#include <stdint.h>

void ata_read_sector(uint32_t lba, uint16_t* target_buffer);
void ata_write_sector(uint32_t lba, const uint16_t* source_buffer);

#endif