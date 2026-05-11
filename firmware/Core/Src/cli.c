#include "cli.h"
#include "main.h"
#include "w25q80.h"
#include "bme68x_driver.h"
#include "bme68x.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

extern UART_HandleTypeDef huart5;

#define CLI_BUF_LEN 256

static char     buf[CLI_BUF_LEN];
static uint32_t buf_pos;
static uint8_t  bme_ready;

/* ------------------------------------------------------------------ */
/* Helpers                                                              */
/* ------------------------------------------------------------------ */

static void prompt(void)
{
    printf("\r\n> ");
}

/* Print a float without requiring -u _printf_float linker flag */
static void print_fixed(const char *label, float val, uint8_t decimals,
                         const char *unit)
{
    int32_t whole = (int32_t)val;
    int32_t frac  = (int32_t)((val - (float)whole) * 100.0f);
    if (frac < 0) frac = -frac;
    if (decimals == 0)
        printf("%s: %ld %s\r\n", label, (long)whole, unit);
    else
        printf("%s: %ld.%02ld %s\r\n", label, (long)whole, (long)frac, unit);
}

/* ------------------------------------------------------------------ */
/* Command handlers                                                     */
/* ------------------------------------------------------------------ */

static void cmd_help(void)
{
    printf("Commands:\r\n");
    printf("  flash read  <addr> <len>      read len bytes from hex addr (hexdump)\r\n");
    printf("  flash erase <addr>            erase 4K sector containing hex addr\r\n");
    printf("  flash write <addr> <b0> ...   write hex bytes to hex addr\r\n");
    printf("  sensor                        one-shot BME688 read (F)\r\n");
    printf("  sensor cont                   continuous reads, any key to stop\r\n");
    printf("  help\r\n");
    printf("\r\n");
    printf("Addresses and bytes are hexadecimal. len is decimal.\r\n");
    printf("Example:\r\n");
    printf("  flash erase 0\r\n");
    printf("  flash write 0 48 65 6C 6C 6F\r\n");
    printf("  flash read  0 16\r\n");
}

static void cmd_flash_read(char *args)
{
    unsigned int addr = 0, len = 0;
    if (sscanf(args, "%x %u", &addr, &len) != 2 || len == 0) {
        printf("Usage: flash read <hex_addr> <len>\r\n");
        return;
    }
    if (len > 4096) {
        printf("Max read length is 4096\r\n");
        return;
    }

    uint8_t row[16];
    for (uint32_t i = 0; i < len; i += 16) {
        uint32_t chunk = (len - i < 16u) ? (len - i) : 16u;
        W25_Read(addr + i, row, chunk);

        printf("%06X:  ", (unsigned)(addr + i));
        for (uint32_t j = 0; j < 16; j++) {
            if (j < chunk) printf("%02X ", row[j]);
            else           printf("   ");
        }
        printf(" |");
        for (uint32_t j = 0; j < chunk; j++)
            printf("%c", (row[j] >= 0x20 && row[j] < 0x7F) ? (char)row[j] : '.');
        printf("|\r\n");
    }
}

static void cmd_flash_erase(char *args)
{
    unsigned int addr = 0;
    if (sscanf(args, "%x", &addr) != 1) {
        printf("Usage: flash erase <hex_addr>\r\n");
        return;
    }
    uint32_t sector = (uint32_t)addr & ~0xFFFu;
    printf("Erasing 4K sector at 0x%06X ... ", (unsigned)sector);
    W25_EraseSector4K(sector);
    printf("done\r\n");
}

static void cmd_flash_write(char *args)
{
    char *p = args;
    char *endp;

    unsigned long addr = strtoul(p, &endp, 16);
    if (endp == p) {
        printf("Usage: flash write <hex_addr> <byte> [byte ...]\r\n");
        return;
    }
    p = endp;

    uint8_t  data[256];
    uint16_t count = 0;
    while (*p && count < 256) {
        while (*p == ' ') p++;
        if (!*p) break;
        char *next;
        unsigned long val = strtoul(p, &next, 16);
        if (next == p) break;
        data[count++] = (uint8_t)(val & 0xFF);
        p = next;
    }

    if (count == 0) {
        printf("No bytes provided\r\n");
        return;
    }

    /* W25_PageProgram must not cross a 256-byte page boundary */
    uint32_t page_end = ((uint32_t)addr & ~0xFFu) + 256u;
    if ((uint32_t)addr + count > page_end) {
        printf("Write would cross 256-byte page boundary — max %lu bytes from 0x%06lX\r\n",
               (unsigned long)(page_end - addr), addr);
        return;
    }

    printf("Writing %u byte(s) at 0x%06lX ... ", (unsigned)count, addr);
    W25_PageProgram((uint32_t)addr, data, count);
    printf("done\r\n");
}

static float to_fahrenheit(float c)
{
    return c * 9.0f / 5.0f + 32.0f;
}

static void print_reading_multiline(struct bme68x_data *d)
{
    print_fixed("Temperature", to_fahrenheit(d->temperature), 2, "F");
    print_fixed("Humidity   ", d->humidity,                   2, "%RH");
    print_fixed("Pressure   ", d->pressure / 100.0f,          2, "hPa");
    if (d->status & BME68X_GASM_VALID_MSK)
        print_fixed("Gas resist ", d->gas_resistance,          0, "ohm");
    else
        printf("Gas resist : not valid (heater stabilizing)\r\n");
}

static void print_reading_inline(struct bme68x_data *d, uint32_t n)
{
    /* Compact single-line format for continuous mode */
    float temp_f = to_fahrenheit(d->temperature);
    int32_t t_w = (int32_t)temp_f,          t_f = (int32_t)((temp_f - t_w) * 100);
    int32_t h_w = (int32_t)d->humidity,     h_f = (int32_t)((d->humidity - h_w) * 100);
    float   hpa = d->pressure / 100.0f;
    int32_t p_w = (int32_t)hpa,             p_f = (int32_t)((hpa - p_w) * 100);

    printf("#%-4lu  %ld.%02ld F  |  %ld.%02ld %%RH  |  %ld.%02ld hPa",
           (unsigned long)n,
           (long)t_w, (long)(t_f < 0 ? -t_f : t_f),
           (long)h_w, (long)(h_f < 0 ? -h_f : h_f),
           (long)p_w, (long)(p_f < 0 ? -p_f : p_f));

    if (d->status & BME68X_GASM_VALID_MSK) {
        int32_t g_w = (int32_t)d->gas_resistance;
        printf("  |  %ld ohm", (long)g_w);
    }
    printf("\r\n");
}

static uint8_t bme_ensure_init(void)
{
    if (bme_ready) return 1;
    printf("Initializing BME688 ...\r\n");
    if (BME688_Init() != 0) { printf("BME688 init failed\r\n"); return 0; }
    bme_ready = 1;
    return 1;
}

static void cmd_sensor(char *args)
{
    uint8_t continuous = (args != NULL && strncmp(args, "cont", 4) == 0);

    if (!bme_ensure_init()) return;

    if (!continuous) {
        /* One-shot */
        struct bme68x_data d;
        printf("Measuring ...\r\n");
        if (BME688_ReadForced(&d) != 0) { printf("Read failed\r\n"); return; }
        print_reading_multiline(&d);
        return;
    }

    /* Continuous — any keypress stops it */
    printf("Continuous mode  (press any key to stop)\r\n");
    printf("#     Temp         Humidity      Pressure       Gas\r\n");
    printf("----  ----------   ----------    ----------     --------\r\n");

    uint32_t count = 0;
    uint8_t  ch;
    for (;;) {
        /* Check for keypress before blocking on measurement */
        if (HAL_UART_Receive(&huart5, &ch, 1, 0) == HAL_OK) break;

        struct bme68x_data d;
        if (BME688_ReadForced(&d) == 0)
            print_reading_inline(&d, ++count);
        else
            printf("Read failed\r\n");

        /* Check again — measurement took ~200 ms, user may have pressed during it */
        if (HAL_UART_Receive(&huart5, &ch, 1, 0) == HAL_OK) break;
    }
    printf("Stopped after %lu reading(s).\r\n", (unsigned long)count);
}

/* ------------------------------------------------------------------ */
/* Line processor                                                       */
/* ------------------------------------------------------------------ */

static void process_line(void)
{
    char *line = buf;
    while (*line == ' ') line++;   /* ltrim */

    /* rtrim */
    int32_t end = (int32_t)strlen(line) - 1;
    while (end >= 0 && (line[end] == ' ' || line[end] == '\r')) line[end--] = '\0';

    if (*line == '\0') { prompt(); return; }

    printf("\r\n");

    if (strcmp(line, "help") == 0) {
        cmd_help();
    } else if (strncmp(line, "flash read ",  11) == 0) {
        cmd_flash_read(line + 11);
    } else if (strncmp(line, "flash erase ", 12) == 0) {
        cmd_flash_erase(line + 12);
    } else if (strncmp(line, "flash write ", 12) == 0) {
        cmd_flash_write(line + 12);
    } else if (strcmp(line, "sensor") == 0) {
        cmd_sensor(NULL);
    } else if (strncmp(line, "sensor ", 7) == 0) {
        cmd_sensor(line + 7);
    } else {
        printf("Unknown command '%s' — type 'help'\r\n", line);
    }

    prompt();
}

/* ------------------------------------------------------------------ */
/* Public API                                                           */
/* ------------------------------------------------------------------ */

void CLI_Init(void)
{
    buf_pos   = 0;
    bme_ready = 0;
    printf("\r\n=== STM32F407 CLI ready ===\r\n");
    printf("Type 'help' for available commands.\r\n");
    prompt();
}

void CLI_Process(void)
{
    uint8_t ch;
    if (HAL_UART_Receive(&huart5, &ch, 1, 0) != HAL_OK)
        return;

    if (ch == '\r' || ch == '\n') {
        if (ch == '\r') {           /* process on CR; ignore the follow-on LF */
            buf[buf_pos] = '\0';
            buf_pos = 0;
            process_line();
        }
    } else if (ch == '\b' || ch == 127) {   /* backspace / DEL */
        if (buf_pos > 0) {
            buf_pos--;
            printf("\b \b");        /* erase character on terminal */
        }
    } else if (ch >= 0x20 && buf_pos < CLI_BUF_LEN - 1) {
        buf[buf_pos++] = ch;
        HAL_UART_Transmit(&huart5, &ch, 1, HAL_MAX_DELAY);   /* local echo */
    }
}
