



#include "bme68x_driver.h"
#include "main.h"
#include <stdio.h>
#include <string.h>

extern I2C_HandleTypeDef hi2c1;

#define BME688_I2C_ADDR   0x77
#define BME688_I2C_ADDR8  (BME688_I2C_ADDR << 1)

static struct bme68x_dev  bme;
static uint8_t            dev_addr    = BME688_I2C_ADDR;
static uint8_t            gas_enabled = 1;

/* ===== Platform glue functions (Bosch driver calls these) ===== */

static BME68X_INTF_RET_TYPE bme_i2c_read(uint8_t reg, uint8_t *data,
                                          uint32_t len, void *intf_ptr)
{
    uint8_t addr = *(uint8_t*)intf_ptr;
    HAL_StatusTypeDef s = HAL_I2C_Mem_Read(&hi2c1, addr << 1, reg,
                                            I2C_MEMADD_SIZE_8BIT,
                                            data, len, HAL_MAX_DELAY);
    return (s == HAL_OK) ? BME68X_INTF_RET_SUCCESS : -1;
}

static BME68X_INTF_RET_TYPE bme_i2c_write(uint8_t reg, const uint8_t *data,
                                           uint32_t len, void *intf_ptr)
{
    uint8_t addr = *(uint8_t*)intf_ptr;
    HAL_StatusTypeDef s = HAL_I2C_Mem_Write(&hi2c1, addr << 1, reg,
                                             I2C_MEMADD_SIZE_8BIT,
                                             (uint8_t*)data, len, HAL_MAX_DELAY);
    return (s == HAL_OK) ? BME68X_INTF_RET_SUCCESS : -1;
}

static void bme_delay_us(uint32_t period, void *intf_ptr)
{
    (void)intf_ptr;
    // Cheap microsecond delay using DWT cycle counter.
    // Requires DWT to be enabled once at boot (see below).
    uint32_t start = DWT->CYCCNT;
    uint32_t ticks = period * (SystemCoreClock / 1000000U);
    while ((DWT->CYCCNT - start) < ticks) { /* spin */ }
}


int8_t BME688_Init(void)
{
    // Enable DWT cycle counter once for microsecond delays.
    // Safe to call multiple times.
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL  |= DWT_CTRL_CYCCNTENA_Msk;

    memset(&bme, 0, sizeof(bme));
    bme.intf       = BME68X_I2C_INTF;
    bme.read       = bme_i2c_read;
    bme.write      = bme_i2c_write;
    bme.delay_us   = bme_delay_us;
    bme.intf_ptr   = &dev_addr;
    bme.amb_temp   = 25;  // approximate ambient temp, °C (used for heater calc)

    int8_t rc = bme68x_init(&bme);
    if (rc != BME68X_OK) {
        printf("bme68x_init failed: %d\r\n", rc);
        return rc;
    }

    // Oversampling + IIR config (good defaults: indoor environment)
    struct bme68x_conf conf = {
        .os_hum  = BME68X_OS_2X,
        .os_temp = BME68X_OS_8X,
        .os_pres = BME68X_OS_4X,
        .filter  = BME68X_FILTER_SIZE_3,
        .odr     = BME68X_ODR_NONE,
    };
    rc = bme68x_set_conf(&conf, &bme);
    if (rc != BME68X_OK) { printf("set_conf: %d\r\n", rc); return rc; }

    // Heater: single-point, 320 °C for 150 ms (standard "indoor air quality" setting)
    struct bme68x_heatr_conf heatr = {
        .enable    = BME68X_ENABLE,
        .heatr_temp = 320,
        .heatr_dur  = 150,
    };
    rc = bme68x_set_heatr_conf(BME68X_FORCED_MODE, &heatr, &bme);
    if (rc != BME68X_OK) { printf("set_heatr: %d\r\n", rc); return rc; }

    printf("BME688 initialized (chip_id=0x%02X, variant=%lu)\r\n",
           bme.chip_id, (unsigned long)bme.variant_id);
    // variant_id: 0 = BME680, 1 = BME688
    return BME68X_OK;
}

int8_t BME688_SetGasEnable(uint8_t enable)
{
    gas_enabled = enable;
    struct bme68x_heatr_conf heatr = {
        .enable     = enable ? BME68X_ENABLE : BME68X_DISABLE,
        .heatr_temp = 320,
        .heatr_dur  = 150,
    };
    return bme68x_set_heatr_conf(BME68X_FORCED_MODE, &heatr, &bme);
}

int8_t BME688_ReadForced(struct bme68x_data *data)
{
    uint8_t n_fields = 0;

    int8_t rc = bme68x_set_op_mode(BME68X_FORCED_MODE, &bme);
    if (rc != BME68X_OK) return rc;

    uint32_t meas_us = bme68x_get_meas_dur(BME68X_FORCED_MODE, NULL, &bme);
    if (gas_enabled) meas_us += 150 * 1000;   /* add heater dwell only when active */
    bme_delay_us(meas_us, NULL);

    rc = bme68x_get_data(BME68X_FORCED_MODE, data, &n_fields, &bme);
    if (rc != BME68X_OK) return rc;
    if (n_fields == 0)   return -1;
    return BME68X_OK;
}

// Heater profile: 10 setpoints, scanned repeatedly in parallel mode.
// Duration values are in "heater cycles" (each cycle ≈ ms based on shared_heatr_dur).
static uint16_t heatr_temp_prof[10] = {320, 100, 100, 100, 200, 200, 200, 320, 320, 320};
static uint16_t heatr_dur_prof[10]  = {5, 2, 10, 30, 5, 5, 5, 5, 5, 5};  // multipliers

int8_t BME688_ConfigParallelHeater(void)
{
    struct bme68x_conf conf = {
        .os_hum  = BME68X_OS_1X,
        .os_temp = BME68X_OS_2X,
        .os_pres = BME68X_OS_16X,
        .filter  = BME68X_FILTER_OFF,
        .odr     = BME68X_ODR_NONE,
    };
    int8_t rc = bme68x_set_conf(&conf, &bme);
    if (rc != BME68X_OK) return rc;

    // Measurement duration of the TPH section (excluding heater) in microseconds,
    // needed so each heater step has the right total duration.
    uint32_t meas_dur_us = bme68x_get_meas_dur(BME68X_PARALLEL_MODE, &conf, &bme);

    struct bme68x_heatr_conf heatr = {
        .enable           = BME68X_ENABLE,
        .heatr_temp_prof  = heatr_temp_prof,
        .heatr_dur_prof   = heatr_dur_prof,
        .profile_len      = 10,
        // Bosch recommends shared_heatr_dur = total_cycle - meas_dur_tph.
        // Cycle length is set in ms; 140 ms is a common default.
        .shared_heatr_dur = 140 - (uint16_t)(meas_dur_us / 1000),
    };
    rc = bme68x_set_heatr_conf(BME68X_PARALLEL_MODE, &heatr, &bme);
    if (rc != BME68X_OK) return rc;

    rc = bme68x_set_op_mode(BME68X_PARALLEL_MODE, &bme);
    return rc;
}

// Read up to 3 fresh fields produced by parallel mode.
// In parallel mode the sensor FIFO may contain 0-3 results per poll.
int8_t BME688_ReadParallel(struct bme68x_data *data, uint8_t *n_fields)
{
    return bme68x_get_data(BME68X_PARALLEL_MODE, data, n_fields, &bme);
}
