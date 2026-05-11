#ifndef W25Q80_H
#define W25Q80_H

#include <stdint.h>

/* Commands */
#define W25_CMD_WRITE_ENABLE    0x06
#define W25_CMD_WRITE_DISABLE   0x04
#define W25_CMD_READ_STATUS1    0x05
#define W25_CMD_READ_STATUS2    0x35
#define W25_CMD_JEDEC_ID        0x9F
#define W25_CMD_READ_DATA       0x03
#define W25_CMD_PAGE_PROGRAM    0x02
#define W25_CMD_SECTOR_ERASE_4K 0x20
#define W25_CMD_CHIP_ERASE      0xC7
#define W25_CMD_POWER_DOWN      0xB9
#define W25_CMD_RELEASE_PD      0xAB

/* Status register bits */
#define W25_STATUS_BUSY         0x01
#define W25_STATUS_WEL          0x02

void    W25_AliveTest(void);
void    W25_Read(uint32_t addr, uint8_t *buf, uint32_t len);
void    W25_PageProgram(uint32_t addr, const uint8_t *data, uint16_t len);
void    W25_EraseSector4K(uint32_t addr);
uint8_t W25_ReadStatus1(void);

#endif /* W25Q80_H */
