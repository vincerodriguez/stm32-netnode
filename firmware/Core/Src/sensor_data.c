#include "sensor_data.h"
#include "bme68x_driver.h"
#include "bme68x.h"
#include "main.h"
#include <string.h>

SensorData_t g_sensor;

static uint8_t  bme_ready;
static uint32_t last_tick;
static uint8_t  gas_pending = 1;   /* desired heater state before driver is up */

void Sensor_Init(void)
{
    memset(&g_sensor, 0, sizeof(g_sensor));
    bme_ready = 0;
    last_tick = 0;
}

void Sensor_Poll(void)
{
    uint32_t now = HAL_GetTick();
    /* First call fires immediately (now - 0 wraps to a large number >= 5000) */
    if (now - last_tick < 1000u) return;
    last_tick = now;

    if (!bme_ready) {
        if (BME688_Init() != 0) return;
        if (!gas_pending) BME688_SetGasEnable(0);  /* apply any pre-init toggle */
        bme_ready = 1;
    }

    struct bme68x_data d;
    if (BME688_ReadForced(&d) != 0) return;

    g_sensor.temp_f        = d.temperature * 9.0f / 5.0f + 32.0f;
    g_sensor.humidity      = d.humidity;
    g_sensor.pressure_hpa  = d.pressure / 100.0f;
    g_sensor.gas_resistance = d.gas_resistance;
    g_sensor.gas_valid     = (d.status & BME68X_GASM_VALID_MSK) ? 1 : 0;
    g_sensor.valid         = 1;
}

void Sensor_SetGasEnable(uint8_t enable)
{
    gas_pending = enable;
    if (bme_ready) BME688_SetGasEnable(enable);
}
