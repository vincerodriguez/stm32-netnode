#include "main.h"
#include "w25q80.h"
#include <stdio.h>
#include <string.h>

extern SPI_HandleTypeDef hspi1;

#define CS_LOW()  HAL_GPIO_WritePin(FLASH_CE_GPIO_Port, FLASH_CE_Pin, GPIO_PIN_RESET)
#define CS_HIGH() HAL_GPIO_WritePin(FLASH_CE_GPIO_Port, FLASH_CE_Pin, GPIO_PIN_SET)

static void spi_tx(uint8_t *buf, uint16_t len)
{
    HAL_SPI_Transmit(&hspi1, buf, len, HAL_MAX_DELAY);
}

static void spi_rx(uint8_t *buf, uint16_t len)
{
    HAL_SPI_Receive(&hspi1, buf, len, HAL_MAX_DELAY);
}

static void cmd3(uint8_t cmd, uint32_t addr)
{
    uint8_t hdr[4] = { cmd, (addr >> 16) & 0xFF, (addr >> 8) & 0xFF, addr & 0xFF };
    spi_tx(hdr, 4);
}

uint8_t W25_ReadStatus1(void)
{
    uint8_t cmd = W25_CMD_READ_STATUS1, sr;
    CS_LOW();
    spi_tx(&cmd, 1);
    spi_rx(&sr, 1);
    CS_HIGH();
    return sr;
}

static void wait_busy(void)
{
    while (W25_ReadStatus1() & W25_STATUS_BUSY) {}
}

static void write_enable(void)
{
    uint8_t cmd = W25_CMD_WRITE_ENABLE;
    CS_LOW();
    spi_tx(&cmd, 1);
    CS_HIGH();
}

void W25_Read(uint32_t addr, uint8_t *buf, uint32_t len)
{
    CS_LOW();
    cmd3(W25_CMD_READ_DATA, addr);
    spi_rx(buf, len);
    CS_HIGH();
}

/* len must be 1–256 and must not cross a 256-byte page boundary */
void W25_PageProgram(uint32_t addr, const uint8_t *data, uint16_t len)
{
    if (len == 0 || len > 256) return;
    write_enable();
    CS_LOW();
    cmd3(W25_CMD_PAGE_PROGRAM, addr);
    spi_tx((uint8_t *)data, len);
    CS_HIGH();
    wait_busy();
}

void W25_EraseSector4K(uint32_t addr)
{
    write_enable();
    CS_LOW();
    cmd3(W25_CMD_SECTOR_ERASE_4K, addr);
    CS_HIGH();
    wait_busy();
}

void W25_AliveTest(void)
{
    CS_HIGH();
    HAL_Delay(10);

    /* 1. JEDEC ID */
    uint8_t cmd = W25_CMD_JEDEC_ID;
    uint8_t id[3] = {0};
    CS_LOW();
    spi_tx(&cmd, 1);
    spi_rx(id, 3);
    CS_HIGH();
    printf("JEDEC: %02X %02X %02X\r\n", id[0], id[1], id[2]);

    if (id[0] == 0xFF || id[0] == 0x00) {
        printf("FAIL: got %02X -- check wiring, CS pin, WP/HOLD tied high, power\r\n", id[0]);
        return;
    }
    if (id[0] != 0xEF) {
        printf("Not a Winbond part (mfr=%02X)\r\n", id[0]);
        return;
    }
    printf("Winbond detected. type=%02X cap=%02X (expect 0x40 0x14 for W25Q80)\r\n", id[1], id[2]);

    /* 2. Status register */
    uint8_t sr1 = W25_ReadStatus1();
    printf("SR1 = 0x%02X (BUSY=%d WEL=%d)\r\n", sr1, sr1 & 1, (sr1 >> 1) & 1);

    /* 3. Erase sector 0, write pattern, read back */
    const char *msg = "Hello W25Q80!";
    uint8_t rb[32] = {0};

    printf("Erasing sector 0...\r\n");
    W25_EraseSector4K(0x000000);

    W25_Read(0x000000, rb, 16);
    printf("After erase: ");
    for (int i = 0; i < 16; i++) printf("%02X ", rb[i]);
    printf("(expect all FF)\r\n");

    printf("Programming...\r\n");
    W25_PageProgram(0x000000, (const uint8_t *)msg, strlen(msg) + 1);

    memset(rb, 0, sizeof(rb));
    W25_Read(0x000000, rb, strlen(msg) + 1);
    printf("Read back: \"%s\"\r\n", (char *)rb);
}
